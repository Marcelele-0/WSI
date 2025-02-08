import hydra
from omegaconf import DictConfig
from src.puzzle.state import *
from src.puzzle.solve_game import solve



@hydra.main(version_base=None, config_path="../config", config_name="main")
def main(cfg):
    puzzle_size = cfg.puzzle.size

    print(cfg.pretty())
    state = generate_random_state(puzzle_size)
    print(state)
    goal_state = generate_goal_state(puzzle_size)
    print(goal_state)

    solve(state, goal_state, cfg.heuristic)
    

if __name__ == "__main__":
    main()

