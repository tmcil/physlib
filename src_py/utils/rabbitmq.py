import json
import os
from uuid import UUID

import pika
from pydantic import BaseModel, Field

from .log import logger

host = os.getenv("RABBITMQ_HOST", "")
username = os.getenv("RABBITMQ_USERNAME", "")
password = os.getenv("RABBITMQ_PASSWORD", "")
execution_queue = os.getenv("RABBITMQ_EXECUTION_QUEUE", "")
log_queue = os.getenv("RABBITMQ_LOG_QUEUE", "")
result_queue = os.getenv("RABBITMQ_RESULT_QUEUE", "")
port = int(os.getenv("RABBITMQ_PORT", ""))


class ResultMessage(BaseModel):
    type: str = Field(default="result")
    execution_id: UUID
    message: str


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
        self.publish_channel = connection.channel()

        if self.publish_channel is None:
            logger.error("Publish channel is not open")
            raise RuntimeError("Publish channel is not open")

        # Declare queues
        self.publish_channel.queue_declare(queue=result_queue, durable=True)
        logger.info("Publishing to queue: %s", result_queue)

    def make_product_available(self, execution_id: UUID, message: str):
        event = ResultMessage(execution_id=execution_id, message=message)

        if self.publish_channel is None:
            logger.error("Publish channel is not open")
            raise RuntimeError("Publish channel is not open")

        self.publish_channel.basic_publish(
            exchange="",
            routing_key=self.result_queue,
            body=event.model_dump_json().encode("utf-8"),
            properties=pika.BasicProperties(
                delivery_mode=2,  # Persistent message
                content_type="application/json",
            ),
        )
