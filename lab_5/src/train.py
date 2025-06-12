import hydra
import numpy as np
from omegaconf import DictConfig
from utils.data import generate_data, normalize_data
import os

@hydra.main(version_base=None, config_path=os.path.join("..", "config"), config_name="config_train")
def main(cfg: DictConfig):
    """Main function to generate and normalize data."""
    # Generate data
    X, y = generate_data(cfg.data.n_samples)
    
    # Normalize data
    X_normalized = normalize_data(X, cfg.data.normalization_method)

    # Print shapes and first few rows of the generated and normalized data
    print(f"Generated data shape: {X.shape}, Labels shape: {y.shape}")
    print(f"First 5 rows of generated data:\n{X[:5]}")
    print(f"First 5 rows of normalized data:\n{X_normalized[:5]}")
    
    # Print L2 norms to verify normalization
    if cfg.data.normalization_method == 'l2':
        l2_norms = np.sqrt(np.sum(X_normalized**2, axis=1))
        print(f"L2 norms after normalization (should be ~1.0): {l2_norms[:5]}")
    elif cfg.data.normalization_method == 'l1':
        l1_norms = np.sum(np.abs(X_normalized), axis=1)
        print(f"L1 norms after normalization (should be ~1.0): {l1_norms[:5]}")

if __name__ == "__main__":
    main()