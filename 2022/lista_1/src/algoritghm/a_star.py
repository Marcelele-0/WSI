import heapq
from puzzle.state import reconstruct_path
from utils.get_neighbors import get_neighbors

def a_star(start, goal_state, heuristic_func, puzzle_size):
    """
    Solve the puzzle using the A* algorithm.
    Returns the path, the number of visited states, and the number of steps.
    """
    frontier = []
    heapq.heappush(frontier, (heuristic_func(start), 0, start))
    came_from = {}  # map: state -> previous state
    move_from = {}  # map: state -> move that led to this state (moved tile)
    cost_so_far = {start: 0}
    visited_count = 0
    steps = 0  # Counter for the number of steps

    while frontier:
        f, g, current = heapq.heappop(frontier)
        visited_count += 1

        if current == goal_state:
            path = reconstruct_path(came_from, current)
            steps = len(path) - 1  # Number of steps is the length of the path minus one
            return path, visited_count, steps

        for neighbor, moved_tile in get_neighbors(current, puzzle_size):
            new_cost = g + 1
            if neighbor not in cost_so_far or new_cost < cost_so_far[neighbor]:
                cost_so_far[neighbor] = new_cost
                priority = new_cost + heuristic_func(neighbor)
                heapq.heappush(frontier, (priority, new_cost, neighbor))
                came_from[neighbor] = current
                move_from[neighbor] = moved_tile

    return None, visited_count, steps
