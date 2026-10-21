import subprocess
import uuid
from pathlib import Path

from .log import logger
from .rabbitmq import RabbitMQClient


def construct_command(
    execution_id: uuid.UUID, block_name: str, block_id: uuid.UUID
) -> list[str]:
    dir = Path("./bin")
    bin = dir / block_name.lower()
    command = [
        f"./{bin!s}",
        f"{execution_id!s}",
        f"{block_id!s}",
    ]

    return command


def run_block(
    execution_id: uuid.UUID,
    block_name: str,
    block_id: uuid.UUID,
    rabbitmq: RabbitMQClient,
) -> int:

    if rabbitmq.publish_channel is None:
        logger.error("Publish channel is not open")
        raise RuntimeError("Publish channel is not open")

    try:
        command = construct_command(execution_id, block_name, block_id)
    except Exception:
        logger.error("Failed to construct commaned")
        raise

    block_id_short = str(block_id).split("-")[-1]

    logger.info(f"RUNNING BLOCK : {block_id_short} {block_name}")
    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        text=True,
        bufsize=1,
    )

    if process.stdout is None:
        logger.error("Failed to capture process output")
        raise RuntimeError("Failed to capture process output")

    for line in process.stdout:
        message = line.rstrip("\r\n")
        if message:
            try:
                rabbitmq.publish_log(execution_id, message)
                logger.info(" ".join(message.split(" ")[2:]))
            except Exception as exc:  # noqa: BLE001
                logger.error(f"Failed to publish log message: {exc}")
                return 1

    return_code = process.wait()

    if return_code == 1:  # Error
        logger.error(f"BLOCK EXITED WITH STATUS ERR: {block_id!s} {block_name}")
        raise subprocess.CalledProcessError(return_code, command)
    elif return_code == 2:  # Warning
        logger.warning(f"BLOCK EXITED WITH STATUS WAR: {block_id!s} {block_name}")

    return return_code
