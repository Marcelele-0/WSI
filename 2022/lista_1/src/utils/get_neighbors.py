_neighbor_cache = {}

def _make_neighbors_map(puzzle_size):
    N = puzzle_size * puzzle_size
    neighbors = {}
    for idx in range(N):
        row, col = divmod(idx, puzzle_size)
        lst = []
        # up/down
        if row > 0:
            lst.append(idx - puzzle_size)
        if row < puzzle_size - 1:
            lst.append(idx + puzzle_size)
        # left/right
        if col > 0:
            lst.append(idx - 1)
        if col < puzzle_size - 1:
            lst.append(idx + 1)
        neighbors[idx] = lst
    return neighbors


def get_neighbors(state, zero_idx, puzzle_size):
    """
    Return list of (neighbor_state, new_zero_idx) by swapping the zero tile.
    Uses a precomputed map of neighbor indices to avoid expensive calculations.
    """
    # Build or retrieve the neighbor map for this puzzle size
    if puzzle_size not in _neighbor_cache:
        _neighbor_cache[puzzle_size] = _make_neighbors_map(puzzle_size)
    neighbors_map = _neighbor_cache[puzzle_size]

    out = []
    lst = list(state)  # single copy
    for ni in neighbors_map[zero_idx]:
        # Swap zero and neighbor
        lst[zero_idx], lst[ni] = lst[ni], lst[zero_idx]
        out.append((tuple(lst), ni))
        # Swap back
        lst[zero_idx], lst[ni] = lst[ni], lst[zero_idx]
    return out
