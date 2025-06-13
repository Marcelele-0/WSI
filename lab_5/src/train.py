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
from models.neural_network import train_neural_network

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
    
    # Train neural network using CUDA propagation
    logger.info("Starting neural network training with CUDA propagation...")
    
    # Training parameters
    epochs = cfg.get('training', {}).get('epochs', 1000)
    learning_rate = cfg.get('training', {}).get('learning_rate', 0.1)
    use_relu = cfg.get('training', {}).get('use_relu', False)
    use_gpu = cfg.get('training', {}).get('use_gpu', False)
    
    # Train the network
    trained_network, training_losses = train_neural_network(
        X_normalized, y, 
        epochs=epochs, 
        learning_rate=learning_rate, 
        use_relu=use_relu,
        use_gpu=use_gpu
    )
    
    logger.info("Neural network training completed successfully!")


if __name__ == "__main__":
    main()