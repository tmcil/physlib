import os
import shutil
import sys
import uuid
from pathlib import Path

from .log import logger


def create_working_dir(execution_id: uuid.UUID, test: bool = False) -> Path:

    if test:
        working_dir = "tests/snapshot/output"
        for res in Path(working_dir).iterdir():
            if res.is_dir():
                shutil.rmtree(res)
        logger.info("Cleared previous snapshot testing outputs")
    else:
        working_dir = os.getenv("EXECUTOR_WORKING_DIR", "")

    if working_dir == "":
        raise RuntimeError("EXECUTOR_WORKING_DIR is not set")

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
    return execution_dir


def check_mnt_connection():
    exists_file = Path(working_dir) / ".exists.txt"
    if not exists_file.exists():
        logger.error(f"Path does not exist: {exists_file!s}")
        raise FileNotFoundError(f"Path does not exist: {exists_file!s}")


def test_copy_inputs(execution_id: uuid.UUID):
    output_dir = Path(f"tests/snapshot/output/{execution_id!s}/inputs/block_graph.json")
    reference_dir = Path(
        f"tests/snapshot/reference/{execution_id!s}/inputs/block_graph.json"
    )
    shutil.copy2(reference_dir, output_dir)
