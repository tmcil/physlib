import sys

from src_py.environment.block_graph import (
    block_graph_get_block,
    block_graph_get_obj_at_port,
)
from src_py.environment.environment import environment_init
from src_py.utils.log import logger
from src_py.utils.postgres import pg_publish_product
from src_py.utils.rabbitmq import RabbitMQClient


def main() -> int:

    # Initialise the environment
    try:
        args, block_graph = environment_init()
        logger.info("Block started: %s", args.program_name)
    except Exception:
        logger.error("Failed to parse command line arguments")
        raise

    # Connect to RabbitMQ
    rabbitmq = RabbitMQClient()
    try:
        rabbitmq.connect()
    except Exception:
        logger.error("Failed to connect to RabbitMQ")
        raise

    # Retrieve the current block from the graph
    try:
        this_block = block_graph_get_block(block_graph, args.block_id)
    except Exception:
        logger.error("Failed the current block (%s) in graph", args.block_id)
        raise

    # Retrieve the input JSON object from the block graph
    try:
        product_type, file_to_upload = block_graph_get_obj_at_port(
            args.execution_id, block_graph, this_block, "product"
        )
    except Exception:
        logger.error("Failed to retrieve file to upload")
        raise

    # Publish the result to RabbitMQ
    try:
        rabbitmq.make_product_available(args.execution_id, file_to_upload)
    except Exception:
        logger.error("Failed to publish result (RabbitMQ): %s", file_to_upload)
        raise

    # Publish the result to postgresDB
    try:
        pg_publish_product(args.execution_id, product_type, file_to_upload)
    except Exception:
        logger.error("Failed to publish result (postgres): %s", file_to_upload)
        raise

    logger.info("Block finished: %s", args.program_name)
    return 0


if __name__ == "__main__":
    try:
        status = main()
    except Exception as exc:  # noqa: BLE001
        logger.error("Failed to run block: %s", exc)
        sys.exit(1)

    sys.exit(status)
