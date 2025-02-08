import hydra

@hydra.main(version_base=None, config_path="../config", config_name="main")
def main(cfg):
    print(cfg.GOAL_STATE)

if __name__ == "__main__":
    main()

