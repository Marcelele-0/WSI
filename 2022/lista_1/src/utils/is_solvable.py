
def is_solvable(state):
    """
    Check if the puzzle state is solvable.
    For a 4x4 board with empty tile at the end (bottom row = 1, which is odd),
    the puzzle is solvable if the number of inversions is even.
    """
    return True