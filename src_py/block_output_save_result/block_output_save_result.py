import sys

from src_py.environment.environment import environment_init
from src_py.utils.log import logger
from src_py.utils.rabbitmq import RabbitMQClient


def main() -> int:

    try:
        args, block_graph = environment_init()
    except Exception:
        logger.error("Failed to parse command line arguments")
        raise

    rabbitmq = RabbitMQClient()
    try:
        rabbitmq.connect()
    except Exception:
        logger.error("Failed to connect to RabbitMQ")
        raise

    return 0


if __name__ == "__main__":
    try:
        status = main()
    except Exception as exc:  # noqa: BLE001
        logger.error("Failed to run block: %s", exc)
        sys.exit(1)

    sys.exit(status)
