import logging
import numpy as np
from typing import Tuple

# Configure logger
logging.basicConfig(level=logging.DEBUG, format='%(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

def generate_data(n_samples: int) -> Tuple[np.ndarray, np.ndarray]:
    """
    Generate synthetic 2D data for classification.

    Args:
        n_samples (int): Number of samples to generate.

    Returns:
        Tuple[np.ndarray, np.ndarray]: A tuple containing:
            - X (np.ndarray): Generated feature data of shape (n_samples, 2).
            - y (np.ndarray): Corresponding labels of shape (n_samples, 1).

    Raises:
        ValueError: If n_samples is not a positive integer.

    Example:
        >>> X, y = generate_data(100)
        >>> print(X.shape, y.shape)
        (100, 2) (100, 1)
    """

    if n_samples <= 0:
        logger.error(f"Invalid n_samples: {n_samples}. Must be positive.")
        raise ValueError("Number of samples must be positive.")

    logger.debug(f"Generating {n_samples} samples of synthetic data")
    
    X = np.random.uniform(-1, 1, (n_samples, 2)).astype(np.float32)
    y = (np.sign(X[:, 0]) == np.sign(X[:, 1])).astype(np.float32).reshape(-1, 1)

    logger.debug(f"Generated data shapes - X: {X.shape}, y: {y.shape}")
    logger.debug(f"Label distribution - Class 0: {np.sum(y == 0)}, Class 1: {np.sum(y == 1)}")

    return X, y


def normalize_data(X: np.ndarray, norm_type: str = 'l2') -> np.ndarray:
    """
    Normalize the data using specified L1 or L2 normalization.

    Args:
        X (np.ndarray): Input data of shape (n_samples, n_features).
        norm_type (str): Type of normalization to apply ('l1', 'l2', or None).

    Returns:
        np.ndarray: Normalized data.

    Raises:
        ValueError: If norm_type is not recognized.
    
    Example:
        >>> X = np.array([[3, 4], [1, 2]])
        >>> normalized_X = normalize_data(X, norm_type='l2')
        >>> print(normalized_X)
        [[0.6 0.8]
         [0.4472136 0.8944272]]
    """
    
    if norm_type is None or norm_type.lower() == 'none':
        logger.debug("No normalization applied")
        return X
    elif norm_type == 'l1':
        logger.debug("Applying L1 normalization")
        # L1 norm: divide each row by sum of absolute values
        l1_norms = np.sum(np.abs(X), axis=1, keepdims=True)
        # Avoid division by zero
        l1_norms = np.where(l1_norms == 0, 1, l1_norms)
        return X / l1_norms
    elif norm_type == 'l2':
        logger.debug("Applying L2 normalization")
        # L2 norm: divide each row by Euclidean norm
        l2_norms = np.sqrt(np.sum(X**2, axis=1, keepdims=True))
        # Avoid division by zero
        l2_norms = np.where(l2_norms == 0, 1, l2_norms)
        return X / l2_norms
    else:
        logger.error(f"Unknown normalization type: {norm_type}")
        raise ValueError(f"Unknown normalization type: {norm_type}. Supported: 'l1', 'l2', None")
    

    

