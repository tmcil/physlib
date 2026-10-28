import uuid

from .log import logger


def block_is_input(block: dict) -> bool:
    ports = block.get("ports", [])
    is_input = True

    for port in ports:
        if port.get("direction", "") == "input":
            is_input = False

    return is_input


def breadth_search_advance(
    block_graph_key: dict, seen_ids: list[uuid.UUID]
) -> list[dict]:
    next_blocks: list = []

    # Append any blocks to the list who connect the seen IDs at an input port
    try:
        [
            next_blocks.append(block)
            for block in block_graph_key
            if any(
                port
                for port in block.get("ports", [])
                if port.get("direction", "") == "input"
                and port.get("connections", [])
                and uuid.UUID(port.get("connections", [])[0].get("blockId", ""))
                in seen_ids
            )
        ]
    except Exception:
        logger.error("Failed to advance search in block grpah")
        raise

    return next_blocks


def determine_block_order(block_graph: dict) -> list[tuple[uuid.UUID, str]]:
    order: list[uuid.UUID] = []

    block_graph_key = block_graph.get("blockGraph", [])
    if block_graph_key == []:
        raise KeyError("blockGraph not found in block_graph.json")

    number_of_blocks = len(block_graph_key)
    seen_blocks = 0
    seen_blocks_this_iter = 0

    # Pass gets all the input blocks, these are ran first
    [
        order.append(uuid.UUID(block.get("blockId", "")))
        for block in block_graph_key
        if block_is_input(block)
    ]

    while seen_blocks < number_of_blocks:
        # Retrieve all the blocks connected to the inputs, one of these must be next
        next_blocks = breadth_search_advance(block_graph_key, order)

        # If a block is not already in the order, and all of its inputs are, then it
        # can be placed next in the order
        [
            order.append(uuid.UUID(block.get("blockId", "")))
            for block in next_blocks
            if uuid.UUID(block.get("blockId", "")) not in order
            and all(
                uuid.UUID(conn.get("blockId", "")) in order
                for port in block.get("ports", [])
                if port.get("direction") == "input"
                for conn in port.get("connections", [])
            )
        ]

        # Check that atleast one new block was found this iteration. If not, then
        # there is a problem with the graph structure
        seen_blocks_this_iter = len(order)
        if (
            seen_blocks_this_iter == seen_blocks
            and seen_blocks_this_iter != number_of_blocks
        ):
            raise SystemError("Order determination failed to make progress")
        else:
            seen_blocks = seen_blocks_this_iter

    # Finally, assign the block names for each block by re-searching the graph
    order_named: list[tuple[uuid.UUID, str]] = []
    [
        order_named.append((block_id, block.get("block", "")))
        for block_id in order
        for block in block_graph_key
        if uuid.UUID(block.get("blockId", "")) == block_id
    ]

    return order_named
