
from functools import lru_cache

@lru_cache(maxsize=None)
def manhattan_distance(state):
    """
    Heuristic: calculate the Manhattan distance for the puzzle.
    """
    distance = 0
    size = int(len(state) ** 0.5)
    target_positions = {value: (divmod(value - 1, size)) for value in range(1, size * size)}  # cache target positions

    for i, value in enumerate(state):
        if value == 0:
            continue
        target_row, target_col = target_positions[value]
        current_row, current_col = divmod(i, size)
        distance += abs(target_row - current_row) + abs(target_col - current_col)

    return distance


def misplaced_tiles(state):
    """
    Heuristic: calculate the number of misplaced tiles for the puzzle.
    """
    return sum(1 for i, value in enumerate(state) if value != 0 and value != i + 1)

@lru_cache(maxsize=None)
def linear_conflict(state):
    """
    Heuristic: calculate the number of linear conflicts for the puzzle.
    """
    size = int(len(state) ** 0.5)
    conflict_count = 0

    # Row conflicts
    for row in range(size):
        row_values = [state[row * size + col] for col in range(size)]
        max_value = -1
        for col in range(size):
            value = row_values[col]
            if value != 0 and (value - 1) // size == row:
                if value > max_value:
                    max_value = value
                else:
                    conflict_count += 2

    # Column conflicts
    for col in range(size):
        col_values = [state[row * size + col] for row in range(size)]
        max_value = -1
        for row in range(size):
            value = col_values[row]
            if value != 0 and (value - 1) % size == col:
                if value > max_value:
                    max_value = value
                else:
                    conflict_count += 2

    return conflict_count
