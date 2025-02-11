import random

from utils.is_solvable import is_solvable


def generate_random_state(puzzle_size):
    """Generate a random state for the puzzle."""

    state = [i for i in range(0, puzzle_size * puzzle_size)]  # we include 0 as the empty tile

    random.shuffle(state)

    state_tuple = tuple(state)

    # we check if the state is solvable
    if not is_solvable(state_tuple):
        # we swap 2 random tiles (not including the empty tile 0)
        i, j = random.sample(range(1, puzzle_size * puzzle_size), 2)
        state[i], state[j] = state[j], state[i]
        state_tuple = tuple(state)

    return state_tuple


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
