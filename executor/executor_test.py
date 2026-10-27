import json
import subprocess
import sys
from pathlib import Path
from uuid import UUID

from pydantic import BaseModel

from .config import write_input_files
from .environment import create_working_dir, test_copy_inputs
from .log import logger
from .order import determine_block_order


def construct_command(execution_id: UUID, block_name: str, block_id: UUID) -> list[str]:
    dir = Path("./build/bin")
    bin = dir / block_name.lower()
    command = [
        f"./{bin!s}",
        f"{execution_id!s}",
        f"{block_id!s}",
    ]

    return command


def test_run_block(
    execution_id: UUID,
    block_name: str,
    block_id: UUID,
) -> int:

    if "output" in block_name.lower():
        logger.info("Skipping output block in tests")
        return 0

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

    return_code = process.wait()

    if return_code == 1:  # Error
        logger.error(f"BLOCK EXITED WITH STATUS ERR: {block_id!s} {block_name}")
        raise subprocess.CalledProcessError(return_code, command)
    elif return_code == 2:  # Warning
        logger.warning(f"BLOCK EXITED WITH STATUS WAR: {block_id!s} {block_name}")

    return return_code


class CmdlineArgs(BaseModel):
    program_name: str
    execution_id: UUID


def parse_cmdline() -> CmdlineArgs:

    try:
        args = CmdlineArgs(program_name=sys.argv[0], execution_id=UUID(sys.argv[1]))

    except Exception:
        logger.error("Invlid command line arguments")
        raise

    return args


def main():

    try:
        logger.info(f"Execution ID: {args.execution_id!s}")

        try:
            execution_dir = create_working_dir(args.execution_id, test=True)
            test_copy_inputs(args.execution_id)
        except Exception as exc:  # noqa: BLE001
            logger.error("Failed to create working directory for execution")
            raise RuntimeError(exc)

        # Load the inputs from the test directory
        try:
            block_graph_f = execution_dir / "inputs" / "block_graph.json"
            with block_graph_f.open("r", encoding="utf-8") as f:
                block_graph = json.load(f)
        except Exception:
            logger.error("Failed to read block graph")
            raise

        # Split the block config into individual JSON objects and write to inputs
        try:
            write_input_files(block_graph, execution_dir)
        except Exception as exc:  # noqa: BLE001
            logger.error("Failed to write input files to disk")
            raise RuntimeError(exc)

        # Read the block list and compute the order of blocks
        try:
            block_order = determine_block_order(block_graph)
        except Exception:
            logger.error("Failed to determine block order")
            raise

        # Run the blocks in order
        logger.info(f"EXECUTION STARTED: {args.execution_id!s}")
        execution_status: int = 0
        for block_id, block_name in block_order:
            try:
                execution_status |= test_run_block(
                    args.execution_id, block_name, block_id
                )
            except Exception:
                logger.error("FAILED TO RUN BLOCK")
                raise
        logger.info(f"EXECUTION FINISHED: {args.execution_id!s}")

    except Exception as exc:
        logger.error(f"Execution Failed: {exc}")
        raise


if __name__ == "__main__":
    try:
        args = parse_cmdline()
    except Exception:  # noqa: BLE001
        logger.error("Test Failed: Could not read cmdline args")
        sys.exit(1)

    try:
        main()
    except Exception:  # noqa: BLE001
        logger.error(f"Test Failed: {args.execution_id!s}")
        sys.exit(1)

    sys.exit(0)
