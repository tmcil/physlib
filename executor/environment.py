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


def create_working_dir(execution_id: uuid.UUID):

    # Clear the execution dir
    execution_dir = Path(working_dir) / str(execution_id)
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

    logger.info(f"Created working directory: {execution_dir!s}")


def check_mnt_connection():
    exists_file = Path(working_dir) / ".exists.txt"
    if not exists_file.exists():
        logger.error(f"Path does not exist: {exists_file!s}")
        raise FileNotFoundError(f"Path does not exist: {exists_file!s}")
