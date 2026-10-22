import sys

from src_py.utils.log import logger


def main() -> int:
    return 0


if __name__ == "__main__":
    logger.info("hello world")
    status: int = main()
    sys.exit(status)
