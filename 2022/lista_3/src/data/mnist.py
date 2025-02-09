import os
import tensorflow as tf
import numpy as np

class DataLoader:
    def __init__(self, batch_size, data_dir="../../data"):
        self.batch_size = batch_size
        self.data_dir = data_dir
        self._ensure_data_dir_exists()
        (self.x_train, self.y_train), (self.x_test, self.y_test) = self._load_data()
        self.x_train = self.x_train.astype("float32") / 255.0
        self.x_test = self.x_test.astype("float32") / 255.0
        self.x_train = self.x_train[..., tf.newaxis]
        self.x_test = self.x_test[..., tf.newaxis]
        self.y_train = self.y_train.astype("int64")
        self.y_test = self.y_test.astype("int64")

    def _ensure_data_dir_exists(self):
        if not os.path.exists(self.data_dir):
            os.makedirs(self.data_dir)

    def _load_data(self):
        data_path = os.path.join(self.data_dir, "mnist.npz")
        if not os.path.exists(data_path):
            (x_train, y_train), (x_test, y_test) = tf.keras.datasets.mnist.load_data()
            with open(data_path, "wb") as f:
                np.savez(f, x_train=x_train, y_train=y_train, x_test=x_test, y_test=y_test)
        else:
            with np.load(data_path) as data:
                x_train = data["x_train"]
                y_train = data["y_train"]
                x_test = data["x_test"]
                y_test = data["y_test"]
        return (x_train, y_train), (x_test, y_test)

    def __iter__(self):
        # Zwracamy dane w partiach
        for i in range(0, len(self.x_train), self.batch_size):
            yield self.x_train[i:i+self.batch_size], self.y_train[i:i+self.batch_size]
