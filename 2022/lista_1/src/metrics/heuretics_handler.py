from metrics.heuretics import linear_conflict, manhattan_distance, misplaced_tiles


def combined_heuristic(state, heuristics):
    """Combine multiple heuristics into a single heuristic value."""
    return sum(heuristic(state) for heuristic in heuristics)


def get_heuristics(cfg):
    """Retrieve the list of heuristics based on the configuration."""
    heuristics = []
    if cfg.heuristics.manhattan_distance:
        heuristics.append(manhattan_distance)
    if cfg.heuristics.misplaced_tiles:
        heuristics.append(misplaced_tiles)
    if cfg.heuristics.linear_conflict:
        heuristics.append(linear_conflict)
    return heuristics


def heuristic_func(state, heuristics):
    """Calculate the heuristic value for the given state."""
    return combined_heuristic(state, heuristics)
