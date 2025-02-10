import tensorflow as tf


class LeNet(tf.keras.Model):
    """
    A LeNet model for MNIST dataset.
    """
    def __init__(self):
        super().__init__()
        self.conv1 = tf.keras.layers.Conv2D(6, 5, activation='tanh', padding='same')
        self.pool1 = tf.keras.layers.AveragePooling2D(pool_size=(2, 2), strides=2)
        self.conv2 = tf.keras.layers.Conv2D(16, 5, activation='tanh', padding='valid')
        self.pool2 = tf.keras.layers.AveragePooling2D(pool_size=(2, 2), strides=2)
        self.flatten = tf.keras.layers.Flatten()
        self.fc1 = tf.keras.layers.Dense(120, activation='tanh')
        self.fc2 = tf.keras.layers.Dense(84, activation='tanh')
        self.fc3 = tf.keras.layers.Dense(10, activation='softmax')

    def call(self, x):
        x = self.conv1(x)
        x = self.pool1(x)
        x = self.conv2(x)
        x = self.pool2(x)
        x = self.flatten(x)
        x = self.fc1(x)
        x = self.fc2(x)
        return self.fc3(x)
