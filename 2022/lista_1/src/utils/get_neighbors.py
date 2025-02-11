def get_neighbors(state, puzzle_size):
    """
    Returns a list of states reachable from the given state in the form of pairs:
      (new_state, moved_tile)
    """
    neighbors = []
    index = state.index(0)
    row, col = divmod(index, puzzle_size)

    # Possible moves: up, down, left, right
    moves = []
    if row > 0:
        moves.append((-1, 0))
    if row < puzzle_size - 1:
        moves.append((1, 0))
    if col > 0:
        moves.append((0, -1))
    if col < puzzle_size - 1:
        moves.append((0, 1))

    for dr, dc in moves:
        new_row, new_col = row + dr, col + dc
        new_index = new_row * puzzle_size + new_col
        new_state = list(state)
        # Swap the blank tile with the tile we are moving
        new_state[index], new_state[new_index] = new_state[new_index], new_state[index]
        moved_tile = state[new_index]  # the tile that we moved
        neighbors.append((tuple(new_state), moved_tile))

    return neighbors
