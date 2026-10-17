import json
import os
import sys
import time
import uuid

import pika

from .environment import check_mnt_connection, create_working_dir
from .log import logger
from .order import determine_block_order
from .postgres import pg_select_json, pg_update_status
from .run import run_block

host = os.getenv("RABBITMQ_HOST", "")
username = os.getenv("RABBITMQ_USERNAME", "")
password = os.getenv("RABBITMQ_PASSWORD", "")
executions_queue = os.getenv("RABBITMQ_EXECUTION_QUEUE", "")
port = int(os.getenv("RABBITMQ_PORT", ""))

if host == "":
    logger.error("Environment var: RABBITMQ_HOST is not set")
    sys.exit(1)
if username == "":
    logger.error("Environment var: RABBITMQ_USERNAME is not set")
    sys.exit(1)
if password == "":
    logger.error("Environment var: RABBITMQ_PASSWORD is not set")
    sys.exit(1)
if executions_queue == "":
    logger.error("Environment var: RABBITMQ_EXECUTION_QUEUE is not set")
    sys.exit(1)
if port == "":
    logger.error("Environment var: RABBITMQ_PORT is not set")
    sys.exit(1)

logger.info("All environment variables are loaded")


def process_message(ch, method, properties, body):
    logger.info("---------------------------------------------------------")
    logger.info("Recieved a new message from the queue")

    # Decode the JSON request body
    try:
        message = json.loads(body.decode("utf-8"))
    except Exception as exc:  # noqa: BLE001
        logger.error(f"Failed to read message from queue: {executions_queue}")
        raise SystemError(exc)

    # Check the UUID is valid
    try:
        execution_id = uuid.UUID(message.get("execution_id", ""))
    except Exception as exc:
        logger.error("Malformed execution ID")
        raise SystemError("Invalid execution ID") from exc

    try:
        logger.info(f"Execution ID: {execution_id}")

        # Process the execution here --------------------------------------

        # Mark the opflow as in progress in Postgres
        try:
            pg_update_status(execution_id, "RUNNING")
        except ValueError as exc:
            logger.error("Failed to update status of execution in db")
            raise SystemError(exc)

        # TODO: Open web socket for log messages

        # Create the file structure for the opflow
        try:
            execution_dir = create_working_dir(execution_id)
        except Exception as exc:  # noqa: BLE001
            logger.error("Failed to create working directory for execution")
            raise SystemError(exc)

        # Load the inputs from Postgres
        try:
            block_graph = pg_select_json(execution_id, "block_graph")
        except Exception as exc:  # noqa: BLE001
            logger.error("Failed to retrieve block graph")
            raise SystemError(exc)
        try:
            block_config = pg_select_json(execution_id, "block_config")
        except Exception as exc:  # noqa: BLE001
            logger.error("Failed to retrieve block config")
            raise SystemError(exc)

        # Write the block graph into the working area
        f_block_graph = execution_dir / "inputs" / "block_graph.json"
        try:
            with f_block_graph.open("w", encoding="utf-8") as f:
                json.dump(block_graph, f, indent=4)
        except Exception as exc:  # noqa: BLE001
            logger.error(f"Failed to write block graph file to: {f_block_graph!s}")
            raise SystemError(exc)

        # Split the block config into individual JSON objects and write to inputs
        block_config_list = block_config.get("blockConfigs", [])
        if block_config_list == []:
            raise SystemError("block_config is an empty list")
        for block in block_config_list:
            try:
                filename = (
                    f"{block['blockName']}_{block['blockId'].split('-')[-1]}.json"
                )
                f_block_config = execution_dir / "inputs" / filename
            except KeyError as exc:
                logger.error("Failed to find blockName/Id in block config")
                raise SystemError(exc)

            try:
                with f_block_config.open("w", encoding="utf-8") as f:
                    json.dump(block["config"], f, indent=4)
            except Exception as exc:  # noqa: BLE001
                logger.error(
                    f"Failed to write block config file to: {f_block_config!s}"
                )
                raise SystemError(exc)

        # block_order = [
        #     (uuid.UUID("db8101a1-b438-4a46-971e-cbc264492de6"), "BLOCK_INPUT_TIM"),
        #     (uuid.UUID("ee512ac5-a494-4ad2-a3a6-ef4822dcd7e8"), "BLOCK_INPUT_TLE"),
        #     (uuid.UUID("e66f001c-2bc4-4f46-99b6-d4c1f138f88c"), "BLOCK_SGP4"),
        # ]

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
                execution_status |= run_block(execution_id, block_name, block_id)
            except Exception:
                logger.error("FAILED TO RUN BLOCK")
                raise
        logger.info(f"EXECUTION FINISHED: {execution_id!s}")

        # TODO: Pipe any log messages through the web socket

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
            raise SystemError(exc)

        # -----------------------------------------------------------------
        ch.basic_ack(delivery_tag=method.delivery_tag)

    except Exception as exc:  # noqa: BLE001
        logger.error(f"Execution Failed: {exc}")

        try:
            pg_update_status(execution_id, "FAILED")
        except ValueError as exc:
            logger.error("Failed to update status of execution in db")
            raise SystemError(exc)

        # Reject malformed messages without requeueing
        ch.basic_nack(
            delivery_tag=method.delivery_tag,
            requeue=False,
        )
    finally:
        logger.info("Consumer ready. Waiting for messages...")


while True:
    # Check persistent volume mount is good
    try:
        check_mnt_connection()
        logger.info("Good connection to persistent volume")
    except FileNotFoundError as exc:
        logger.error(f"Bad connection to persistent volume: {exc}")
        sys.exit(1)

    # Connect with rabbitmq
    try:
        credentials = pika.PlainCredentials(username, password)
        connection = pika.BlockingConnection(
            pika.ConnectionParameters(
                host=host,
                port=port,
                credentials=credentials,
                heartbeat=30,
                blocked_connection_timeout=60,
            )
        )
        channel = connection.channel()
        channel.queue_declare(queue=executions_queue, durable=True)

        channel.basic_qos(prefetch_count=1)
        channel.basic_consume(
            queue=executions_queue,
            on_message_callback=process_message,
            auto_ack=False,
        )

        logger.info(f"{executions_queue} consumer ready. Waiting for messages...")
        channel.start_consuming()

    except KeyboardInterrupt:
        sys.exit(1)
    except Exception as exc:  # noqa: BLE001
        logger.error(f"RabbitMQ connection error: {exc}")
        time.sleep(5)
