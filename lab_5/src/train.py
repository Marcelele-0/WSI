
# Standard library imports
import logging
import os
import sys

# Related third party imports
import hydra
import numpy as np
from omegaconf import DictConfig

# Local application imports
from utils.data import generate_data, normalize_data

# Add path to CUDA module
sys.path.append(os.path.join(os.path.dirname(__file__), "propagation", "build"))
import cuda_nn  # type: ignore

@hydra.main(version_base=None, config_path=os.path.join("..", "config"), config_name="config_train")
def main(cfg: DictConfig):
    """Main function to generate and normalize data."""
    # Configure logger after Hydra initialization
    logging.basicConfig(level=logging.INFO, format='%(levelname)s - %(message)s', force=True)
    logger = logging.getLogger(__name__)
    
    logger.info("Starting training with config: n_samples=%d, normalization=%s", 
                cfg.data.n_samples, cfg.data.normalization_method)
    
    # Generate data
    X, y = generate_data(cfg.data.n_samples)
    
    # Normalize data
    X_normalized = normalize_data(X, cfg.data.normalization_method)


if __name__ == "__main__":
    main()