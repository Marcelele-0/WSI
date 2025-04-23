import random

from utils.is_solvable import is_solvable


def generate_random_state(puzzle_size):
    """Generate a random, solvable state with 0 (empty tile) in the bottom-right corner."""
    N = puzzle_size * puzzle_size
    tiles = list(range(1, N))  # [1, 2, …, N-1]

    while True:
        random.shuffle(tiles)
        state = tiles + [0]      # dokładamy 0 na koniec
        if is_solvable(state):
            return tuple(state)

def generate_goal_state(grid_size):
    """Generate the goal state for the puzzle."""
    goal_state = [i for i in range(1, grid_size * grid_size)]
    goal_state.append(0)

    return tuple(goal_state)


def reconstruct_path(came_from, current_state):
    """Reconstruct the path from the start state to the current state."""
    total_path = [current_state]
    while current_state in came_from.keys():
        current_state = came_from[current_state]
        total_path.insert(0, current_state)
    return total_path


def print_state(state, size):
    """Print the state in a readable format."""
    if len(state) != size * size:
        raise ValueError("State size does not match the specified puzzle size.")

    border = "+" + "----+" * size
    for i in range(size):
        print(border)
        for j in range(size):
            value = state[i * size + j]
            if value == 0:
                print("|    ", end="")
            else:
                print(f"| {value:2} ", end="")
        print("|")
    print(border)
