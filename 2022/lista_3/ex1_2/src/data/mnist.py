import os

import numpy as np
import tensorflow as tf


class DataLoader:
    def __init__(self, batch_size=32, data_dir="../../data"):
        self.batch_size = batch_size
        self.data_dir = os.path.abspath(data_dir)
        self.data_path = os.path.join(self.data_dir, "mnist.npz")

        self._ensure_data_dir_exists()
        self._download_if_needed()
        self._load_data()

    def _ensure_data_dir_exists(self):
        """Creates the directory if it does not exist."""
        os.makedirs(self.data_dir, exist_ok=True)

    def _is_valid_dataset(self):
        """Checks if the file exists and contains valid data."""
        if not os.path.exists(self.data_path):
            return False
        try:
            with np.load(self.data_path) as data:
                return all(key in data for key in ["x_train", "y_train", "x_test", "y_test"])
        except Exception as e:
            print(f"Corrupt dataset detected: {e}")
            return False

    def _download_if_needed(self):
        """Downloads the MNIST dataset if the file is missing or corrupted."""
        if not self._is_valid_dataset():
            print(f"Downloading MNIST dataset to {self.data_path}...")
            (x_train, y_train), (x_test, y_test) = tf.keras.datasets.mnist.load_data()
            np.savez(
                self.data_path, x_train=x_train, y_train=y_train, x_test=x_test, y_test=y_test
            )
            print("Download complete!")

    def _load_data(self):
        """Loads data from the file."""
        print(f"Loading dataset from {self.data_path}...")
        with np.load(self.data_path) as data:
            self.x_train, self.y_train = data["x_train"], data["y_train"]
            self.x_test, self.y_test = data["x_test"], data["y_test"]

        # Normalize and add channel dimension
        self.x_train = np.expand_dims(self.x_train.astype("float32") / 255.0, axis=-1)
        self.x_test = np.expand_dims(self.x_test.astype("float32") / 255.0, axis=-1)

    def _map_fn(self, x, y):
        return x, y

    def get_dataset(self, train=True):
        """Returns a `tf.data.Dataset` for training or testing with optimizations."""
        x, y = (self.x_train, self.y_train) if train else (self.x_test, self.y_test)
        dataset = tf.data.Dataset.from_tensor_slices((x, y))

        if train:
            dataset = dataset.shuffle(len(x))

        dataset = (
            dataset.batch(self.batch_size)
            .cache()
            .map(self._map_fn, num_parallel_calls=tf.data.experimental.AUTOTUNE)
            .prefetch(tf.data.experimental.AUTOTUNE)
        )

        return dataset
