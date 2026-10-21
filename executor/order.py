import uuid

from .log import logger


def node_is_input(node: dict) -> bool:
    block = node.get("data", {})
    ports = block.get("ports", [])
    is_input = True

    for port in ports:
        if port.get("direction", "") == "input":
            is_input = False

    return is_input


def breadth_search_advance(
    nodes: list[dict], edges: list[dict], seen_ids: list[uuid.UUID]
) -> list[tuple[uuid.UUID, dict]]:
    next_blocks: list = []

    ids: list[uuid.UUID] = [uuid.UUID(node.get("id", "")) for node in nodes]
    blocks: list[dict] = [node.get("data", {}) for node in nodes]

    # Append any blocks to the list who connect the seen IDs at an input port
    try:
        [
            next_blocks.append((id, block))
            for id, block in zip(ids, blocks)
            if any(
                port
                for port in block.get("ports", [])
                if port.get("direction", "") == "input"
                and any(
                    uuid.UUID(edge.get("source", "")) in seen_ids
                    for edge in edges
                    if uuid.UUID(edge.get("target", "")) == id
                )
            )
        ]
    except Exception:
        logger.error("Failed to advance search in block grpah")
        raise

    return next_blocks


def determine_block_order(block_graph: dict) -> list[tuple[uuid.UUID, str]]:
    order: list[uuid.UUID] = []
    nodes: list[dict] = block_graph.get("nodes", [])
    edges: list[dict] = block_graph.get("edges", [])

    seen_nodes = 0
    seen_nodes_this_iter = 0

    # Pass gets all the input nodes, these are ran first
    [
        order.append(uuid.UUID(node.get("id", "")))
        for node in nodes
        if node_is_input(node)
    ]

    # Run the search until all nodes have been seen
    while seen_nodes < len(nodes):
        # Retrieve all the blocks connected to the inputs, one of these must be next
        next_nodes = breadth_search_advance(nodes, edges, order)

        # If a block is not already in the order, and all of its inputs are, then it
        # can be placed next in the order
        [
            order.append(id)
            for id, block in next_nodes
            if id not in order
            and all(
                edge.get("source", "") in order
                for edge in edges
                if edge.get("target", "") == id
            )
        ]

        # Check that atleast one new block was found this iteration. If not, then
        # there is a problem with the graph structure
        seen_nodes_this_iter = len(order)
        if seen_nodes_this_iter == seen_nodes and seen_nodes_this_iter != len(nodes):
            raise SystemError("Order determination failed to make progress")
        else:
            seen_nodes = seen_nodes_this_iter

    # Finally, assign the block names for each block by re-searching the graph
    order_named: list[tuple[uuid.UUID, str]] = []
    [
        order_named.append((id, node.get("data", {}).get("block", "")))
        for id in order
        for node in nodes
        if uuid.UUID(node.get("id", "")) == id
    ]

    logger.info("Determined block order from block graph")
    return order_named
