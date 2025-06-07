# -*- coding: utf-8 -*-
"""
dbscan.py

Custom DBSCAN implementation with automatic strategy selection based on dataset size.
Supports different optimization levels for small, medium, and large datasets.

This implementation is designed for educational purposes and provides transparency
in the clustering process while maintaining reasonable performance.
"""

import numpy as np
from sklearn.neighbors import NearestNeighbors
from collections import deque
from typing import Optional, Tuple


class DBSCAN:
    """
    Custom DBSCAN implementation with automatic optimization strategy selection.
    
    Automatically chooses the most appropriate clustering strategy based on dataset size:
    - Small datasets (<2000 samples): Pairwise distance matrix approach
    - Medium datasets (2000-10000 samples): Optimized neighbor search
    - Large datasets (>10000 samples): Memory-efficient batch processing
    
    Parameters
    ----------
    eps : float, default=0.5
        The maximum distance between two samples for them to be considered
        as in the same neighborhood (radius parameter).
        
    min_samples : int, default=4
        The number of samples (or total weight) in a neighborhood for a point
        to be considered as a core point. This includes the point itself.
        
    random_state : int, RandomState instance or None, default=None
        Random state for reproducible results.
        
    verbose : int, default=0
        Controls the verbosity of the clustering process.
        - 0: Silent
        - 1: Basic progress information
        - 2: Detailed progress information
        
    Attributes
    ----------
    labels_ : ndarray of shape (n_samples,)
        Cluster labels for each point in the dataset. Noisy samples are given
        the label -1.
        
    n_clusters_ : int
        Number of clusters found (excluding noise).
        
    core_sample_indices_ : ndarray of shape (n_core_samples,)
        Indices of core samples.
        
    Examples
    --------
    >>> from dbscan import DBSCAN
    >>> import numpy as np
    >>> X = np.random.rand(100, 2)
    >>> dbscan = DBSCAN(eps=0.3, min_samples=5)
    >>> dbscan.fit(X)
    >>> labels = dbscan.labels_
    >>> n_clusters = dbscan.n_clusters_
    """
    
    def __init__(self, eps: float = 0.5, min_samples: int = 4, 
                 random_state: Optional[int] = None, verbose: int = 0):
        self.eps = eps
        self.min_samples = min_samples
        self.random_state = random_state
        self.verbose = verbose
        
        # Fitted attributes
        self.labels_ = None
        self.n_clusters_ = 0
        self.core_sample_indices_ = None
        
    def fit(self, X: np.ndarray) -> 'DBSCAN':
        """
        Perform DBSCAN clustering on the input data.
        
        Automatically selects the most appropriate clustering strategy based
        on the size of the dataset.
        
        Parameters
        ----------
        X : ndarray of shape (n_samples, n_features)
            Training data to cluster.
            
        Returns
        -------
        self : DBSCAN
            Fitted estimator.
        """
        X = np.asarray(X, dtype=np.float64)
        n_samples = X.shape[0]
        
        if self.verbose >= 1:
            print(f"Clustering {n_samples} samples with DBSCAN")
            print(f"Parameters: eps={self.eps}, min_samples={self.min_samples}")
        
        # Automatic strategy selection based on dataset size
        if n_samples < 2000:
            if self.verbose >= 1:
                print("Using pairwise distance approach (small dataset)")
            return self._fit_pairwise(X)
        elif n_samples <= 10000:
            if self.verbose >= 1:
                print("Using optimized approach (medium dataset)")
            return self._fit_optimized(X)
        else:
            if self.verbose >= 1:
                print("Using memory-efficient approach (large dataset)")
            return self._fit_memory_efficient(X)
    
    def fit_predict(self, X: np.ndarray) -> np.ndarray:
        """
        Perform clustering and return cluster labels.
        
        Parameters
        ----------
        X : ndarray of shape (n_samples, n_features)
            Input data to cluster.
            
        Returns
        -------
        labels : ndarray of shape (n_samples,)
            Cluster labels. Noisy samples are labeled as -1.
        """
        return self.fit(X).labels_
    
    def _fit_pairwise(self, X: np.ndarray) -> 'DBSCAN':
        """
        DBSCAN implementation using pairwise distance matrix.
        Suitable for small datasets where memory is not a constraint.
        """
        n_samples = X.shape[0]
        
        # Compute pairwise squared distances
        if self.verbose >= 2:
            print("Computing pairwise distance matrix...")
        distances_sq = self._pairwise_distances_squared(X)
        
        # Find core points
        if self.verbose >= 2:
            print("Identifying core points...")
        neighbor_counts = np.sum(distances_sq <= self.eps ** 2, axis=1)
        core_mask = neighbor_counts >= self.min_samples
        
        # Initialize labels and visited array
        labels = np.full(n_samples, -1, dtype=int)
        visited = np.zeros(n_samples, dtype=bool)
        cluster_id = 0
        
        # Find all neighbors for each point
        neighbors_list = []
        for i in range(n_samples):
            neighbors = np.where(distances_sq[i] <= self.eps ** 2)[0]
            neighbors_list.append(neighbors)
        
        # Cluster expansion
        core_indices = np.where(core_mask)[0]
        for idx_counter, i in enumerate(core_indices):
            if visited[i]:
                continue
                
            if self.verbose >= 2 and idx_counter % 100 == 0:
                print(f"Processing core point {idx_counter}/{len(core_indices)}")
            
            # Start new cluster
            visited[i] = True
            labels[i] = cluster_id
            
            # Expand cluster using BFS
            queue = deque(neighbors_list[i].tolist())
            seen = set(neighbors_list[i].tolist())
            seen.add(i)
            
            while queue:
                j = queue.popleft()
                if not visited[j]:
                    visited[j] = True
                    labels[j] = cluster_id
                    
                    if core_mask[j]:  # If j is also a core point
                        for neighbor in neighbors_list[j]:
                            if neighbor not in seen:
                                seen.add(neighbor)
                                queue.append(neighbor)
            
            cluster_id += 1
        
        self.labels_ = labels
        self.n_clusters_ = cluster_id
        self.core_sample_indices_ = core_indices
        
        if self.verbose >= 1:
            print(f"Found {cluster_id} clusters and {np.sum(labels == -1)} noise points")
        
        return self
    
    def _fit_optimized(self, X: np.ndarray) -> 'DBSCAN':
        """
        Optimized DBSCAN using NearestNeighbors for medium-sized datasets.
        Balances memory usage and computational efficiency.
        """
        n_samples = X.shape[0]
        
        # Pre-compute all neighbors using NearestNeighbors
        if self.verbose >= 2:
            print("Building neighbor index...")
        nbrs = NearestNeighbors(
            radius=self.eps,
            algorithm="ball_tree",
            n_jobs=-1
        ).fit(X)
        neighbors_list = nbrs.radius_neighbors(X, return_distance=False)
        
        # Vectorized core point detection
        if self.verbose >= 2:
            print("Identifying core points...")
        core_mask = np.array([len(neighbors) >= self.min_samples 
                             for neighbors in neighbors_list])
        
        # Initialize clustering arrays
        labels = np.full(n_samples, -1, dtype=int)
        visited = np.zeros(n_samples, dtype=bool)
        cluster_id = 0
        
        # Cluster expansion
        core_indices = np.where(core_mask)[0]
        for idx_counter, i in enumerate(core_indices):
            if visited[i]:
                continue
                
            if self.verbose >= 2 and idx_counter % 100 == 0:
                print(f"Expanding cluster {cluster_id}, core point {idx_counter}/{len(core_indices)}")
            
            # Start new cluster
            visited[i] = True
            labels[i] = cluster_id
            
            # Expand cluster using BFS
            queue = deque(neighbors_list[i].tolist())
            seen = set(neighbors_list[i].tolist())
            seen.add(i)
            
            while queue:
                j = queue.popleft()
                if not visited[j]:
                    visited[j] = True
                    labels[j] = cluster_id
                    
                    if core_mask[j]:  # If j is also a core point
                        for neighbor in neighbors_list[j]:
                            if neighbor not in seen:
                                seen.add(neighbor)
                                queue.append(neighbor)
            
            cluster_id += 1
        
        self.labels_ = labels
        self.n_clusters_ = cluster_id
        self.core_sample_indices_ = core_indices
        
        if self.verbose >= 1:
            print(f"Found {cluster_id} clusters and {np.sum(labels == -1)} noise points")
        
        return self
    
    def _fit_memory_efficient(self, X: np.ndarray, batch_size: int = 1000) -> 'DBSCAN':
        """
        Memory-efficient DBSCAN that processes data in batches.
        Suitable for large datasets where memory is limited.
        """
        n_samples = X.shape[0]
        
        # Build neighbor index
        if self.verbose >= 2:
            print("Building neighbor index...")
        nbrs = NearestNeighbors(
            radius=self.eps,
            algorithm="auto",
            n_jobs=-1
        ).fit(X)
        
        # Process neighbors in batches to save memory
        core_mask = np.zeros(n_samples, dtype=bool)
        neighbors_cache = {}
        
        if self.verbose >= 2:
            print("Computing neighbors in batches...")
        
        for start_idx in range(0, n_samples, batch_size):
            end_idx = min(start_idx + batch_size, n_samples)
            batch_indices = np.arange(start_idx, end_idx)
            batch_neighbors = nbrs.radius_neighbors(X[batch_indices], return_distance=False)
            
            for i, neighbors in enumerate(batch_neighbors):
                actual_idx = start_idx + i
                core_mask[actual_idx] = len(neighbors) >= self.min_samples
                neighbors_cache[actual_idx] = neighbors.tolist()
            
            if self.verbose >= 2 and (start_idx // batch_size) % 10 == 0:
                print(f"Processed batch {start_idx // batch_size + 1}/{(n_samples - 1) // batch_size + 1}")
        
        # Initialize clustering arrays
        labels = np.full(n_samples, -1, dtype=int)
        visited = np.zeros(n_samples, dtype=bool)
        cluster_id = 0
        
        # Cluster expansion
        core_indices = np.where(core_mask)[0]
        for idx_counter, i in enumerate(core_indices):
            if visited[i]:
                continue
                
            if self.verbose >= 2 and idx_counter % 100 == 0:
                print(f"Expanding cluster {cluster_id}, core point {idx_counter}/{len(core_indices)}")
            
            # Start new cluster
            visited[i] = True
            labels[i] = cluster_id
            
            # Expand cluster using BFS
            queue = deque(neighbors_cache[i])
            seen = set(neighbors_cache[i])
            seen.add(i)
            
            while queue:
                j = queue.popleft()
                if not visited[j]:
                    visited[j] = True
                    labels[j] = cluster_id
                    
                    if core_mask[j]:  # If j is also a core point
                        # Load neighbors if not in cache
                        if j not in neighbors_cache:
                            neighbors_cache[j] = nbrs.radius_neighbors(
                                X[j:j+1], return_distance=False
                            )[0].tolist()
                        
                        for neighbor in neighbors_cache[j]:
                            if neighbor not in seen:
                                seen.add(neighbor)
                                queue.append(neighbor)
            
            cluster_id += 1
        
        self.labels_ = labels
        self.n_clusters_ = cluster_id
        self.core_sample_indices_ = core_indices
        
        if self.verbose >= 1:
            print(f"Found {cluster_id} clusters and {np.sum(labels == -1)} noise points")
        
        return self
    
    def _pairwise_distances_squared(self, X: np.ndarray) -> np.ndarray:
        """
        Compute pairwise squared Euclidean distances efficiently.
        
        Uses the identity: ||x - y||² = ||x||² + ||y||² - 2⟨x,y⟩
        
        Parameters
        ----------
        X : ndarray of shape (n_samples, n_features)
            Input data.
            
        Returns
        -------
        distances_sq : ndarray of shape (n_samples, n_samples)
            Pairwise squared distances.
        """
        norms_sq = np.sum(X * X, axis=1).reshape(-1, 1)
        gram_matrix = X.dot(X.T)
        distances_sq = norms_sq + norms_sq.T - 2 * gram_matrix
        
        # Ensure non-negative values (numerical stability)
        np.maximum(distances_sq, 0, out=distances_sq)
        
        return distances_sq
    
    def get_params(self, deep: bool = True) -> dict:
        """
        Get parameters for this estimator.
        
        Parameters
        ----------
        deep : bool, default=True
            If True, will return the parameters for this estimator and
            contained subobjects that are estimators.
            
        Returns
        -------
        params : dict
            Parameter names mapped to their values.
        """
        return {
            'eps': self.eps,
            'min_samples': self.min_samples,
            'random_state': self.random_state,
            'verbose': self.verbose
        }
    
    def set_params(self, **params) -> 'DBSCAN':
        """
        Set the parameters of this estimator.
        
        Parameters
        ----------
        **params : dict
            Estimator parameters.
            
        Returns
        -------
        self : DBSCAN
            Estimator instance.
        """
        for key, value in params.items():
            if hasattr(self, key):
                setattr(self, key, value)
            else:
                raise ValueError(f"Invalid parameter {key}")
        return self
