# -*- coding: utf-8 -*-
"""
kmeans.py

Custom implementation of K-Means clustering algorithm with k-means++ initialization.
"""
import numpy as np
from tqdm import tqdm


class KMeans:
    """
    Custom K-Means clustering implementation with k-means++ initialization.
    
    Parameters
    ----------
    n_clusters : int, default=8
        Number of clusters to form.
    max_iter : int, default=300
        Maximum number of iterations for the k-means algorithm.
    tol : float, default=1e-4
        Tolerance for convergence.
    random_state : int, default=None
        Random state for reproducibility.
    """
    
    def __init__(self, n_clusters=8, max_iter=300, tol=1e-4, random_state=None):
        """
        Initialize KMeans clustering.
        
        Parameters:
        -----------
        n_clusters : int, default=8
            Number of clusters to form.
        max_iter : int, default=300
            Maximum number of iterations for the k-means algorithm.
        tol : float, default=1e-4
            Tolerance for convergence.
        random_state : int, default=None
            Random state for reproducibility.
        """
        self.n_clusters = n_clusters
        self.max_iter = max_iter
        self.tol = tol
        self.random_state = random_state
        
        # Attributes set during fitting
        self.cluster_centers_ = None
        self.labels_ = None
        self.inertia_ = None
        self.n_iter_ = None
        
    def _initialize_centroids_plus_plus(self, X):
        """
        Initialize centroids using the k-means++ algorithm.
        
        Parameters
        ----------
        X : array-like of shape (n_samples, n_features)
            Training data.
        
        Returns
        -------
        centroids : ndarray of shape (n_clusters, n_features)
            Initial centroids.
        """
        n_samples, n_features = X.shape
        rng = np.random.RandomState(self.random_state)
        
        centroids = np.empty((self.n_clusters, n_features), dtype=X.dtype)
        
        # Choose first centroid randomly
        first_idx = rng.randint(0, n_samples)
        centroids[0] = X[first_idx]
        
        # Choose remaining centroids
        closest_dist_sq = np.full(n_samples, np.inf)
        
        for c in range(1, self.n_clusters):
            # Calculate distances to the newest centroid
            diff = X - centroids[c - 1]
            dist_to_new_centroid = np.sum(diff * diff, axis=1)
            
            # Update minimum distances
            closest_dist_sq = np.minimum(closest_dist_sq, dist_to_new_centroid)
            
            # Choose next centroid with probability proportional to squared distance
            total_dist = np.sum(closest_dist_sq)
            if total_dist == 0.0:
                next_idx = rng.randint(0, n_samples)
            else:
                probs = closest_dist_sq / total_dist
                next_idx = rng.choice(n_samples, p=probs)
            
            centroids[c] = X[next_idx]
            
        return centroids
    
    def _assign_clusters(self, X, centroids):
        """
        Assign each point to the nearest centroid.
        
        Parameters
        ----------
        X : array-like of shape (n_samples, n_features)
            Training data.
        centroids : ndarray of shape (n_clusters, n_features)
            Current centroids.
        
        Returns
        -------
        labels : ndarray of shape (n_samples,)
            Cluster labels for each point.
        """
        n_samples = X.shape[0]
        distances = np.empty((n_samples, self.n_clusters))
        
        for i, centroid in enumerate(centroids):
            diff = X - centroid
            distances[:, i] = np.sum(diff * diff, axis=1)
            
        return np.argmin(distances, axis=1)
    
    def _update_centroids(self, X, labels):
        """
        Update centroids based on current cluster assignments.
        
        Parameters
        ----------
        X : array-like of shape (n_samples, n_features)
            Training data.
        labels : ndarray of shape (n_samples,)
            Current cluster labels.
        
        Returns
        -------
        new_centroids : ndarray of shape (n_clusters, n_features)
            Updated centroids.
        """
        n_features = X.shape[1]
        new_centroids = np.zeros((self.n_clusters, n_features))
        
        for k in range(self.n_clusters):
            mask = (labels == k)
            if np.any(mask):
                new_centroids[k] = np.mean(X[mask], axis=0)
            else:
                # If cluster is empty, reinitialize randomly
                rng = np.random.RandomState(self.random_state)
                random_idx = rng.randint(0, X.shape[0])
                new_centroids[k] = X[random_idx]
                
        return new_centroids
    
    def _calculate_inertia(self, X, labels, centroids):
        """
        Calculate within-cluster sum of squares (inertia).
        
        Parameters
        ----------
        X : array-like of shape (n_samples, n_features)
            Training data.
        labels : ndarray of shape (n_samples,)
            Cluster labels.
        centroids : ndarray of shape (n_clusters, n_features)
            Cluster centroids.
        
        Returns
        -------
        inertia : float
            Sum of squared distances of samples to their closest centroid.
        """
        inertia = 0.0
        for k in range(self.n_clusters):
            mask = (labels == k)
            if np.any(mask):
                diff = X[mask] - centroids[k]
                inertia += np.sum(diff * diff)
        return inertia
    
    def fit(self, X, verbose=True):
        """
        Compute k-means clustering.
        
        Parameters
        ----------
        X : array-like of shape (n_samples, n_features)
            Training data to cluster.
        verbose : bool, default=True
            Whether to show progress bar.
        
        Returns
        -------
        self : object
            Fitted estimator.
        """
        X = np.asarray(X)
        n_samples, n_features = X.shape
        
        # Initialize centroids using k-means++
        centroids = self._initialize_centroids_plus_plus(X)
        labels = np.full(n_samples, -1, dtype=int)
        
        # Main k-means loop
        iterator = tqdm(range(self.max_iter), desc="K-Means iterations") if verbose else range(self.max_iter)
        
        for iteration in iterator:
            # Assign points to nearest centroids
            new_labels = self._assign_clusters(X, centroids)
            
            # Update centroids
            new_centroids = self._update_centroids(X, new_labels)
            
            # Check for convergence
            centroid_shifts = np.sqrt(np.sum((centroids - new_centroids) ** 2, axis=1))
            if np.max(centroid_shifts) <= self.tol:
                centroids = new_centroids
                labels = new_labels
                self.n_iter_ = iteration + 1
                break
                
            centroids = new_centroids
            labels = new_labels
        else:
            self.n_iter_ = self.max_iter
        
        # Store results
        self.cluster_centers_ = centroids
        self.labels_ = labels
        self.inertia_ = self._calculate_inertia(X, labels, centroids)
        
        return self
    
    def predict(self, X):
        """
        Predict the closest cluster each sample in X belongs to.
        
        Parameters
        ----------
        X : array-like of shape (n_samples, n_features)
            New data to predict.
        
        Returns
        -------
        labels : ndarray of shape (n_samples,)
            Index of the cluster each sample belongs to.
        """
        if self.cluster_centers_ is None:
            raise ValueError("This KMeans instance is not fitted yet.")
            
        return self._assign_clusters(X, self.cluster_centers_)
    
    def fit_predict(self, X, verbose=True):
        """
        Compute cluster centers and predict cluster index for each sample.
        
        Parameters
        ----------
        X : array-like of shape (n_samples, n_features)
            Training data to cluster.
        verbose : bool, default=True
            Whether to show progress bar.
        
        Returns
        -------
        labels : ndarray of shape (n_samples,)
            Index of the cluster each sample belongs to.
        """
        return self.fit(X, verbose=verbose).labels_


def run_multiple_kmeans(X, n_clusters, n_runs=5, random_state_base=42, verbose=True):
    """
    Run k-means multiple times and return the best result (lowest inertia).
    
    Parameters
    ----------
    X : array-like of shape (n_samples, n_features)
        Training data to cluster.
    n_clusters : int
        Number of clusters.
    n_runs : int, default=5
        Number of different runs to perform.
    random_state_base : int, default=42
        Base random state (each run uses random_state_base + run_number).
    verbose : bool, default=True
        Whether to show progress.
    
    Returns
    -------
    best_kmeans : KMeans
        The fitted KMeans instance with the lowest inertia.
    all_inertias : list
        List of inertias from all runs.
    """
    best_kmeans = None
    best_inertia = np.inf
    all_inertias = []
    
    if verbose:
        print(f"Running K-Means {n_runs} times for k={n_clusters}...")
    
    for run in range(n_runs):
        random_state = random_state_base + run if random_state_base is not None else None
        
        kmeans = KMeans(n_clusters=n_clusters, random_state=random_state)
        kmeans.fit(X, verbose=False)
        
        all_inertias.append(kmeans.inertia_)
        
        if kmeans.inertia_ < best_inertia:
            best_inertia = kmeans.inertia_
            best_kmeans = kmeans
            
        if verbose:
            print(f"  Run {run+1}/{n_runs}: inertia = {kmeans.inertia_:.2f}")
    
    if verbose:
        print(f"  Best inertia: {best_inertia:.2f}")
        
    return best_kmeans, all_inertias
