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
    # Configure logger after Hydra initialization - log to both console and file
    import hydra
    from hydra.core.hydra_config import HydraConfig
    
    # Get Hydra's output directory and job name
    hydra_cfg = HydraConfig.get()
    output_dir = hydra_cfg.runtime.output_dir
    job_name = hydra_cfg.job.name
    
    # Configure logging to write to both console and file
    log_file = os.path.join(output_dir, f"{job_name}.log")
    
    # Clear any existing handlers
    logging.getLogger().handlers.clear()
    
    # Configure logging with both file and console handlers
    logging.basicConfig(
        level=logging.INFO,
        format='%(levelname)s - %(message)s',
        handlers=[
            logging.FileHandler(log_file),
            logging.StreamHandler()
        ],
        force=True
    )
    
    logger = logging.getLogger(__name__)
    logger.info(f"Starting training with config: n_samples={cfg.data.n_samples}, normalization={cfg.data.normalization_method}")
    logger.info(f"Logs will be saved to: {log_file}")
    
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