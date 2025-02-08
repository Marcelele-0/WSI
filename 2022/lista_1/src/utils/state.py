import random

from utils.is_solvable import is_solvable


def generate_random_state(size):
    """
    Generate a random state for the puzzle.
    """

    state = [i for i in range(1, size*size - 1)] # we skip the empty tile

    # we add the empty tile at the end
    state.append(0) 

    random.shuffle(state)

    state_tuple = tuple(state)

    # we check if the state is solvable
    if not is_solvable(state_tuple):
        # we swap 2 random tiles (not including the empty tile 0)
        i, j = random.sample(range(1, size*size), 2)
        state[i], state[j] = state[j], state[i]
        state_tuple = tuple(state)

    return state_tuple

def generate_goal_state(size):
    """
    Generate the goal state for the puzzle.
    """
    goal_state = (i for i in range(size*size - 1))
    goal_state.append(0)

    return tuple(i for i in range(size*size))

def print_state(state, size):
    """
    Print the state in a readable format.
    """
    for i in range(size):
        for j in range(size):
            print(state[i*size + j], end=" ")
        print()

