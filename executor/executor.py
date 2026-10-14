import json
import os
import sys
import time
import uuid

import pika

from .environment import check_mnt_connection, create_working_dir
from .log import logger
from .postgres import pg_update_status

host = os.getenv("RABBITMQ_HOST", "")
username = os.getenv("RABBITMQ_USERNAME", "")
password = os.getenv("RABBITMQ_PASSWORD", "")
queue = os.getenv("RABBITMQ_QUEUE", "")

RABBITMQ_PORT = 5672

if host == "":
    logger.error("Environment var: RABBITMQ_HOST is not set")
    sys.exit(1)
if username == "":
    logger.error("Environment var: RABBITMQ_USERNAME is not set")
    sys.exit(1)
if password == "":
    logger.error("Environment var: RABBITMQ_PASSWORD is not set")
    sys.exit(1)
if queue == "":
    logger.error("Environment var: RABBITMQ_QUEUE is not set")
    sys.exit(1)

logger.info("All environment variables are loaded")


def process_message(ch, method, properties, body):
    logger.info("Recieved a new message from the queue")
    try:
        # Decode the JSON request body
        try:
            message = json.loads(body.decode("utf-8"))
        except Exception as exc:  # noqa: BLE001
            logger.error(f"Failed to read message from queue: {queue}")
            raise SystemError(exc)

        # Check the UUID is valid
        try:
            execution_id = uuid.UUID(message.get("execution_id", ""))
        except Exception as exc:
            logger.error("Malformed execution ID")
            raise SystemError("Invalid execution ID") from exc

        logger.info(f"Execution ID: {execution_id}")

        # Process the execution here --------------------------------------

        # TODO: Mark the opflow as in progress in Postgres
        try:
            pg_update_status(execution_id, "RUNNING")
        except ValueError as exc:
            logger.error("Failed to update status of execution in db")
            raise SystemError(exc)

        # TODO: Open web socket for log messages

        # Create the file structure for the opflow
        try:
            create_working_dir(execution_id)
        except Exception as exc:  # noqa: BLE001
            logger.error("Failed to create working directory for execution")
            raise SystemError(exc)

        # TODO: Load the inputs from Postgres

        # TODO: Write the block graph Postgres into the working area
        # TODO: Write the split block configs into the working area

        # TODO: Read the block list and compute the order of blocks

        # TODO: Run the blocks in order
        # TODO: Pipe any log messages through the web socket

        try:
            pg_update_status(execution_id, "SUCCESS")
        except ValueError as exc:
            logger.error("Failed to update status of execution in db")
            raise SystemError(exc)

        # -----------------------------------------------------------------
        ch.basic_ack(delivery_tag=method.delivery_tag)

    except Exception as exc:  # noqa: BLE001
        logger.error(f"Execution Failed: {exc}")

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
                port=RABBITMQ_PORT,
                credentials=credentials,
                heartbeat=30,
                blocked_connection_timeout=60,
            )
        )
        channel = connection.channel()
        channel.queue_declare(queue=queue, durable=True)

        channel.basic_qos(prefetch_count=1)
        channel.basic_consume(
            queue=queue,
            on_message_callback=process_message,
            auto_ack=False,
        )

        logger.info("Consumer ready. Waiting for messages...")
        channel.start_consuming()

    except KeyboardInterrupt:
        sys.exit(1)
    except Exception as exc:  # noqa: BLE001
        logger.error(f"RabbitMQ connection error: {exc}")
        time.sleep(5)
