import json
from pathlib import Path

from .log import logger


def write_input_files(block_graph: dict, execution_dir: Path):

    for block in block_graph.get("blockGraph", []):
        blockId = block.get("blockId", "")
        blockName = block.get("block", "")

        # Read the config from the graph
        config = get_config(block)
        if config == {}:
            logger.error(f"No config found for block: {block.get('name', '')}")
            raise SystemError("No config found for block")

        # Construct the filepath
        try:
            filename = f"{blockName}_{blockId.split('-')[-1]}.json"
            f_block_config = execution_dir / "inputs" / filename
        except KeyError as exc:
            logger.error("Failed to find blockName/Id in block config")
            raise SystemError(exc)

        # Write the file
        try:
            with f_block_config.open("w", encoding="utf-8") as f:
                json.dump(config, f, indent=4)
        except Exception as exc:  # noqa: BLE001
            logger.error(f"Failed to write block config file to: {f_block_config!s}")
            raise SystemError(exc)


def get_config(block: dict) -> dict:

    output_config = {}
    output_config.update(
        (config_type_entry.get("name", ""), config_type_entry.get("value", ""))
        for config_type in block.get("config", {}).values()
        for config_type_entry in config_type
    )

    return output_config
