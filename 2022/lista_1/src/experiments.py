import csv
import io
import os
from contextlib import redirect_stdout
from multiprocessing import Pool

from hydra import initialize, compose

from main import solve  

heuristics_list = [
    {"manhattan_distance": True, "linear_conflict": False, "misplaced_tiles": False},
    {"manhattan_distance": True, "linear_conflict": True, "misplaced_tiles": False},
    {"manhattan_distance": False, "linear_conflict": True, "misplaced_tiles": False},
]

num_runs_per_config = 1
num_of_processes = 10
output_file = "results4.csv"


def run_experiment(heuristics, run_id):
    heur_name = "+".join([k for k, v in heuristics.items() if v])

    overrides = [
        f"heuristics.manhattan_distance={heuristics['manhattan_distance']}",
        f"heuristics.linear_conflict={heuristics['linear_conflict']}",
        f"heuristics.misplaced_tiles={heuristics['misplaced_tiles']}",
    ]

    with initialize(config_path="../config", version_base=None):
        cfg = compose(config_name="main", overrides=overrides)

    f = io.StringIO()
    with redirect_stdout(f):
        visited, steps = solve(cfg, verbose=False)

    if visited is not None and steps is not None:
        return {
            "heuristic": heur_name,
            "run": run_id,
            "visited": visited,
            "steps": steps
        }
    else:
        return None


def run_experiments():
    os.chdir(os.path.dirname(__file__) + "/..")  # ustaw katalog roboczy na główny katalog projektu

    with open(output_file, 'a', newline='') as csvfile:
        fieldnames = ["heuristic", "run", "visited", "steps"]
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()

        # Tworzenie puli procesów do równoległego wykonania eksperymentów
        with Pool(processes=num_of_processes) as pool:
            results = []

            # Używamy multiprocessing.Pool do przetwarzania eksperymentów równolegle
            for heuristics in heuristics_list:
                for run_id in range(1, num_runs_per_config + 1):
                    results.append(pool.apply_async(run_experiment, (heuristics, run_id)))

            # Odbieranie wyników
            for result in results:
                experiment_result = result.get()
                if experiment_result:
                    writer.writerow(experiment_result)

    print(f"\nZapisano wyniki do: {output_file}")


if __name__ == "__main__":
    run_experiments()
