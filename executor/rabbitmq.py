import json
import os
import sys

import pika
from uuid import UUID

from .log import logger

host = os.getenv("RABBITMQ_HOST", "")
username = os.getenv("RABBITMQ_USERNAME", "")
password = os.getenv("RABBITMQ_PASSWORD", "")
execution_queue = os.getenv("RABBITMQ_EXECUTION_QUEUE", "")
log_queue = os.getenv("RABBITMQ_LOG_QUEUE", "")
result_queue = os.getenv("RABBITMQ_RESULT_QUEUE", "")
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
if execution_queue == "":
    logger.error("Environment var: RABBITMQ_EXECUTION_QUEUE is not set")
    sys.exit(1)
if log_queue == "":
    logger.error("Environment var: RABBITMQ_LOG_QUEUE is not set")
    sys.exit(1)
if result_queue == "":
    logger.error("Environment var: RABBITMQ_RESULT_QUEUE is not set")
    sys.exit(1)
if port == "":
    logger.error("Environment var: RABBITMQ_PORT is not set")
    sys.exit(1)


class RabbitMQClient:
    def __init__(self):
        self.connection = None
        self.publish_channel = None
        self.consumer_channel = None

        self.execution_queue = execution_queue
        self.log_queue = log_queue
        self.result_queue = result_queue

    def connect(self):

        # Access RabbitMQ service
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

        # Create channels
        self.consumer_channel = connection.channel()
        self.publish_channel = connection.channel()

        if self.publish_channel is None:
            logger.error("Publish channel is not open")
            raise RuntimeError("Publish channel is not open")
        if self.consumer_channel is None:
            logger.error("Consumer channel is not open")
            raise RuntimeError("Consumer channel is not open")

        # Declare queues
        self.consumer_channel.queue_declare(queue=execution_queue, durable=True)
        logger.info("Consuming RabbitMQ queue: %s", execution_queue)
        self.publish_channel.queue_declare(queue=log_queue, durable=True)
        logger.info("Publishing to RabbitMQ queue: %s", log_queue)
        self.publish_channel.queue_declare(queue=result_queue, durable=True)
        logger.info("Publishing to RabbitMQ queue: %s", result_queue)

        logger.info("Connected to RabbitMQ")

    def publish_log(self, execution_id: UUID, message: str):
        event = {
            "type": "log",
            "execution_id": str(execution_id),
            "message": message,
        }

        if self.publish_channel is None:
            logger.error("Publish channel is not open")
            raise RuntimeError("Publish channel is not open")

        self.publish_channel.basic_publish(
            exchange="",
            routing_key=self.log_queue,
            body=json.dumps(event).encode("utf-8"),
            properties=pika.BasicProperties(
                delivery_mode=2,  # Persistent message
                content_type="application/json",
            ),
        )
