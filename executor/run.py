import subprocess
import uuid
from pathlib import Path

from .log import logger


def construct_command(
    execution_id: uuid.UUID, block_name: str, block_id: uuid.UUID
) -> str:
    dir = Path("./bin")
    bin = dir / block_name.lower()
    command = f"./{bin!s} {execution_id!s} {block_id!s}"

    return command


def run_block(execution_id: uuid.UUID, block_name: str, block_id: uuid.UUID) -> int:

    try:
        command = construct_command(execution_id, block_name, block_id)
    except Exception:
        logger.error("Failed to construct commaned")
        raise

    logger.info(f"RUNNING BLOCK : {block_id!s} {block_name}")
    exit_code = subprocess.run(command, shell=True, check=False)
    logger.info(f"BLOCK FINISHED: {block_id!s} {block_name}")

    if exit_code.returncode == 1:  # Error
        logger.error(f"BLOCK EXITED WITH STATUS ERR: {block_id!s} {block_name}")
        raise subprocess.CalledProcessError(exit_code.returncode, command)
    elif exit_code.returncode == 2:  # Warning
        logger.warning(f"BLOCK EXITED WITH STATUS WAR: {block_id!s} {block_name}")

    return exit_code.returncode
