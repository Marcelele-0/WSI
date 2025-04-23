import heapq
import time

from puzzle.state import reconstruct_path
from utils.get_neighbors import get_neighbors
from utils.is_solvable import is_solvable


def a_star(initial_state, goal_state, heuristic_func, puzzle_size):
    """Solve the puzzle using the A* algorithm.

    Returns the path, the number of visited states, and the number of steps.
    """
    start_time = time.time()  # Begin timing the whole algorithm

    frontier = []
    heapq.heappush(frontier, (heuristic_func(initial_state), 0, initial_state))
    came_from = {}  # map: state -> previous state
    move_from = {}  # map: state -> move that led to this state (moved tile)
    cost_so_far = {initial_state: 0}
    visited = set()  # Set to track visited states
    visited_count = 0
    steps = 0  # Counter for the number of steps

    if not is_solvable(initial_state):
        raise ValueError("Initial state is not solvable.")
    
    print ("Initial state is solvable.")

    while frontier:
        f, g, current = heapq.heappop(frontier)
        
        if current in visited:
            continue
        
        visited.add(current)
        visited_count += 1

        if current == goal_state:
            path = reconstruct_path(came_from, current)
            steps = len(path) - 1
            total_time = time.time() - start_time
            print(f"Total time: {total_time:.4f} seconds")
            return path, visited_count, steps

        zero_idx = current.index(0)
        for neighbor, moved_tile in get_neighbors(current, zero_idx, puzzle_size):
            new_cost = g + 1
            if neighbor not in cost_so_far or new_cost < cost_so_far[neighbor]:
                cost_so_far[neighbor] = new_cost
                priority = new_cost + heuristic_func(neighbor)
                heapq.heappush(frontier, (priority, new_cost, neighbor))
                came_from[neighbor] = current
                move_from[neighbor] = moved_tile


    total_time = time.time() - start_time  # End timing if no solution is found
    print(f"Total time: {total_time:.4f} seconds")
    return None, visited_count, steps
