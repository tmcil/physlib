import json
import os
import sys
import time
import uuid

import pika

from .config import write_input_files
from .environment import check_mnt_connection, create_working_dir
from .log import logger
from .order import determine_block_order
from .postgres import pg_select_json, pg_update_status, pg_clear_results
from .rabbitmq import RabbitMQClient, execution_queue
from .run import run_block

# Connect to rabbitMQ
rabbitmq = RabbitMQClient()
try:
    rabbitmq.connect()
except Exception as exc:  # noqa: BLE001
    logger.error(f"RabbitMQ connection error: {exc}")
    sys.exit(1)


def process_message(ch, method, properties, body):
    logger.info("---------------------------------------------------------")
    logger.info("Recieved a new message from the queue")

    # Decode the JSON request body
    try:
        message = json.loads(body.decode("utf-8"))
    except Exception as exc:  # noqa: BLE001
        logger.error("Failed to read message from queue")
        raise RuntimeError(exc)

    # Check the UUID is valid
    try:
        execution_id = uuid.UUID(message.get("execution_id", ""))
    except Exception as exc:
        logger.error("Malformed execution ID")
        raise RuntimeError("Invalid execution ID") from exc

    try:
        logger.info(f"Execution ID: {execution_id}")

        # Process the execution here --------------------------------------

        # Mark the opflow as in progress in Postgres
        try:
            pg_update_status(execution_id, "RUNNING")
        except ValueError as exc:
            logger.error("Failed to update status of execution in db")
            raise RuntimeError(exc)

        # Clear any previous results from the database
        try:
            pg_clear_results(execution_id)
        except Exception as exc:  # noqa: BLE001
            logger.error("Failed to clear previous results")
            raise RuntimeError(exc)

        # Create the file structure for the opflow
        try:
            execution_dir = create_working_dir(execution_id)
        except Exception as exc:  # noqa: BLE001
            logger.error("Failed to create working directory for execution")
            raise RuntimeError(exc)

        # Load the inputs from Postgres
        try:
            block_graph = pg_select_json(execution_id, "block_graph")
        except Exception as exc:  # noqa: BLE001
            logger.error("Failed to retrieve block graph")
            raise RuntimeError(exc)

        # Write the block graph into the working area
        f_block_graph = execution_dir / "inputs" / "block_graph.json"
        try:
            with f_block_graph.open("w", encoding="utf-8") as f:
                json.dump(block_graph, f, indent=4)
        except Exception as exc:  # noqa: BLE001
            logger.error(f"Failed to write block graph file to: {f_block_graph!s}")
            raise RuntimeError(exc)

        # Split the block config into individual JSON objects and write to inputs
        try:
            write_input_files(block_graph, execution_dir)
        except Exception as exc:  # noqa: BLE001
            logger.error("Failed to write input files to disk")
            raise RuntimeError(exc)

        # Read the block list and compute the order of blocks
        try:
            block_order = determine_block_order(block_graph)
        except Exception:
            logger.error("Failed to determine block order")
            raise

        # Run the blocks in order
        logger.info(f"EXECUTION STARTED: {execution_id!s}")
        execution_status: int = 0
        for block_id, block_name in block_order:
            try:
                execution_status |= run_block(
                    execution_id, block_name, block_id, rabbitmq
                )
            except Exception:
                logger.error("FAILED TO RUN BLOCK")
                raise
        logger.info(f"EXECUTION FINISHED: {execution_id!s}")

        # Select the correct status, it can never be FAILED here, since 1's are
        # raised inside run_block()
        match execution_status:
            case 0:
                final_status = "SUCCESS"
            case 2:
                final_status = "WARNING"
            case _:
                final_status = "UNKNOWN"

        try:
            pg_update_status(execution_id, final_status)
        except ValueError as exc:
            logger.error("Failed to update status of execution in db")
            raise RuntimeError(exc)

        # -----------------------------------------------------------------
        ch.basic_ack(delivery_tag=method.delivery_tag)

    except Exception as exc:  # noqa: BLE001
        logger.error(f"Execution Failed: {exc}")

        try:
            pg_update_status(execution_id, "FAILED")
        except ValueError as exc:
            logger.error("Failed to update status of execution in db")
            raise RuntimeError(exc)

        # Reject malformed messages without requeueing
        ch.basic_nack(
            delivery_tag=method.delivery_tag,
            requeue=False,
        )
    finally:
        logger.info("Consumer ready. Waiting for messages...")


def main():
    # Check persistent volume mount is good
    try:
        check_mnt_connection()
        logger.info("Good connection to persistent volume")
    except FileNotFoundError as exc:
        logger.error(f"Bad connection to persistent volume: {exc}")
        sys.exit(1)

    # Consume the execution queue
    try:
        while True:
            if rabbitmq.consumer_channel is None:
                logger.error("Consumer channel is not open")
                raise RuntimeError("Consumer channel is not open")

            rabbitmq.consumer_channel.basic_qos(prefetch_count=1)
            rabbitmq.consumer_channel.basic_consume(
                queue=execution_queue,
                on_message_callback=process_message,
                auto_ack=False,
            )
            logger.info(f"{execution_queue} consumer ready. Waiting for messages...")
            rabbitmq.consumer_channel.start_consuming()

    except KeyboardInterrupt:
        sys.exit(1)
    except Exception as exc:  # noqa: BLE001
        logger.error(f"RabbitMQ connection error: {exc}")
        time.sleep(5)


if __name__ == "__main__":
    main()
    sys.exit(0)
