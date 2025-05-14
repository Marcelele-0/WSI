# main.py
import hydra
from omegaconf import DictConfig

from algoritghm.a_star import a_star
from metrics.heuretics_handler import combined_heuristic, get_heuristics
from puzzle.state import generate_goal_state, generate_random_state, print_state


def solve(cfg: DictConfig, verbose=True):
    puzzle_size = cfg.puzzle.size

    state = generate_random_state(puzzle_size)
    if verbose:
        print("Initial state:")
        print_state(state, puzzle_size)

    goal_state = generate_goal_state(puzzle_size)
    if verbose:
        print("Goal state:")
        print_state(goal_state, puzzle_size)

    heuristics = get_heuristics(cfg)

    if heuristics:
        path, visited_count, steps = a_star(
            state, goal_state, lambda state: combined_heuristic(state, heuristics), puzzle_size
        )
        if path:
            if verbose:
                print("Solution found!")
                for step in path:
                    print_state(step, puzzle_size)
                    print()
                print(f"Total states visited: {visited_count}")
                print(f"Number of steps: {steps}")
            return visited_count, steps
        else:
            if verbose:
                print("No solution found.")
            return None, None
    else:
        if verbose:
            print("No heuristic function selected.")
        return None, None


@hydra.main(version_base=None, config_path="../config", config_name="main")
def main(cfg: DictConfig):
    solve(cfg)


if __name__ == "__main__":
    main()
