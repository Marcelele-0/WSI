class PuzzleConfig:
    """
    Configuration class for the puzzle solver.
    Contains settings for puzzle size and available heuristics.
    """
    def __init__(self):
        # Puzzle dimensions
        self.PUZZLE_SIZE = 4  # 4x4 puzzle
        self.GRID_SIZE = self.PUZZLE_SIZE * self.PUZZLE_SIZE
        
        # Available heuristic functions
        self.HEURISTICS = {
            'manhattan': 'Manhattan Distance',
            'misplaced': 'Misplaced Tiles',
            'linear_conflict': 'Linear Conflict + Manhattan Distance',
            'pattern_database': 'Pattern Database'
        }
        
        # Default heuristic
        self.DEFAULT_HEURISTIC = 'manhattan'
    
    def is_valid_heuristic(self, heuristic):
        """
        Check if the provided heuristic is valid.
        
        Parameters:
            heuristic (str): Name of the heuristic to check
            
        Returns:
            bool: True if heuristic exists, False otherwise
        """
        return heuristic in self.HEURISTICS
    
    def get_available_heuristics(self):
        """
        Get list of available heuristics with their descriptions.
        
        Returns:
            dict: Dictionary of heuristic names and their descriptions
        """
        return self.HEURISTICS

# Create a global instance of the configuration
puzzle_config = PuzzleConfig() 