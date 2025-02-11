def is_solvable(state):
    """Check if the puzzle state is solvable. For a 4x4 board with empty tile at the end (bottom
    row = 1, which is odd), the puzzle is solvable if the number of inversions is even.

    Parameters:
        state (list): The current state of the puzzle.

    Returns:
        bool: True if the puzzle is solvable, False otherwise.
    """
    # we skip the empty tile
    inversion_count = 0

    # we scan the state list and count the inversions
    state_list = [x for x in state if x != 0]
    for i in range(len(state_list)):
        for j in range(i + 1, len(state_list)):
            if state_list[i] > state_list[j]:
                inversion_count += 1
    return inversion_count % 2 == 0
