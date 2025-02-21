import hydra
from algoritghm.a_star import a_star
from metrics.heuretics_handler import combined_heuristic, get_heuristics, heuristic_func
from omegaconf import DictConfig
from puzzle.state import generate_goal_state, generate_random_state, print_state


@hydra.main(version_base=None, config_path="../config", config_name="main")
def main(cfg: DictConfig):
    puzzle_size = cfg.puzzle.size

    state = generate_random_state(puzzle_size)
    print("Initial state:")
    print_state(state, puzzle_size)

    goal_state = generate_goal_state(puzzle_size)
    print("Goal state:")
    print_state(goal_state, puzzle_size)

    heuristics = get_heuristics(cfg)

    if heuristics:
        path, visited_count, steps = a_star(
            state, goal_state, lambda state: heuristic_func(state, heuristics), puzzle_size
        )
        if path:
            print("Solution found!")
            for step in path:
                print_state(step, puzzle_size)
                print()
            print(f"Total states visited: {visited_count}")
            print(f"Number of steps: {steps}")
        else:
            print("No solution found.")
    else:
        print("No heuristic function selected.")


if __name__ == "__main__":
    main()
