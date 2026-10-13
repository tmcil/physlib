import os
import shutil
import sys
import uuid
from pathlib import Path

from .log import logger

working_dir = os.getenv("EXECUTOR_WORKING_DIR", "")

if working_dir == "":
    logger.error("Environment var: EXECUTOR_WORKING_DIR is not set")
    sys.exit(1)


def create_working_dir(message: dict):

    # Check the UUID is valid
    execution_id_str = message.get("execution_id", "")
    try:
        uuid.UUID(execution_id_str)
    except (ValueError, TypeError, AttributeError) as exc:
        logger.error(f"Malformed execution ID: {execution_id_str}")
        raise ValueError("Invalid execution ID") from exc

    # Clear the execution dir
    execution_dir = Path(working_dir) / execution_id_str
    if execution_dir.exists():
        shutil.rmtree(execution_dir)
    execution_dir.mkdir(parents=True)

    # Create the subdirs
    inputs_dir = execution_dir / "inputs"
    inputs_dir.mkdir(parents=True)
    outputs_dir = execution_dir / "outputs"
    outputs_dir.mkdir(parents=True)
    files_dir = execution_dir / "files"
    files_dir.mkdir(parents=True)
    logs_dir = execution_dir / "logs"
    logs_dir.mkdir(parents=True)
