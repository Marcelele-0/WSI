import tensorflow as tf


class InceptionModule(tf.keras.layers.Layer):
    """Inception module for GoogleNet."""

    def __init__(
        self,
        filters_1x1,
        filters_3x3_reduce,
        filters_3x3,
        filters_5x5_reduce,
        filters_5x5,
        filters_pool_proj,
    ):
        super().__init__()
        self.conv1x1 = tf.keras.layers.Conv2D(filters_1x1, (1, 1), padding="same", activation=None)
        self.bn1x1 = tf.keras.layers.BatchNormalization()
        self.conv3x3_reduce = tf.keras.layers.Conv2D(
            filters_3x3_reduce, (1, 1), padding="same", activation=None
        )
        self.bn3x3_reduce = tf.keras.layers.BatchNormalization()
        self.conv3x3 = tf.keras.layers.Conv2D(filters_3x3, (3, 3), padding="same", activation=None)
        self.bn3x3 = tf.keras.layers.BatchNormalization()
        self.conv5x5_reduce = tf.keras.layers.Conv2D(
            filters_5x5_reduce, (1, 1), padding="same", activation=None
        )
        self.bn5x5_reduce = tf.keras.layers.BatchNormalization()
        self.conv5x5 = tf.keras.layers.Conv2D(filters_5x5, (5, 5), padding="same", activation=None)
        self.bn5x5 = tf.keras.layers.BatchNormalization()
        self.pool_proj = tf.keras.layers.Conv2D(
            filters_pool_proj, (1, 1), padding="same", activation=None
        )
        self.bn_pool_proj = tf.keras.layers.BatchNormalization()
        self.pool = tf.keras.layers.MaxPooling2D((3, 3), strides=(1, 1), padding="same")

    def call(self, x):
        conv1x1 = tf.nn.relu(self.bn1x1(self.conv1x1(x)))
        conv3x3 = tf.nn.relu(self.bn3x3(self.conv3x3(self.bn3x3_reduce(self.conv3x3_reduce(x)))))
        conv5x5 = tf.nn.relu(self.bn5x5(self.conv5x5(self.bn5x5_reduce(self.conv5x5_reduce(x)))))
        pool_proj = tf.nn.relu(self.bn_pool_proj(self.pool_proj(self.pool(x))))
        return tf.concat([conv1x1, conv3x3, conv5x5, pool_proj], axis=-1)


class GoogleNet(tf.keras.Model):
    """GoogleNet model."""

    def __init__(self):
        super().__init__()
        self.conv1 = tf.keras.layers.Conv2D(
            64, (7, 7), strides=(2, 2), padding="same", activation=None
        )
        self.bn1 = tf.keras.layers.BatchNormalization()
        self.pool1 = tf.keras.layers.MaxPooling2D((3, 3), strides=(2, 2), padding="same")
        self.conv2 = tf.keras.layers.Conv2D(192, (3, 3), padding="same", activation=None)
        self.bn2 = tf.keras.layers.BatchNormalization()
        self.pool2 = tf.keras.layers.MaxPooling2D((3, 3), strides=(2, 2), padding="same")
        self.inception3a = InceptionModule(64, 96, 128, 16, 32, 32)
        self.inception3b = InceptionModule(128, 128, 192, 32, 96, 64)
        self.pool3 = tf.keras.layers.MaxPooling2D((3, 3), strides=(2, 2), padding="same")
        self.inception4a = InceptionModule(192, 96, 208, 16, 48, 64)
        self.inception4b = InceptionModule(160, 112, 224, 24, 64, 64)
        self.inception4c = InceptionModule(128, 128, 256, 24, 64, 64)
        self.inception4d = InceptionModule(112, 144, 288, 32, 64, 64)
        self.inception4e = InceptionModule(256, 160, 320, 32, 128, 128)
        self.pool4 = tf.keras.layers.MaxPooling2D((3, 3), strides=(2, 2), padding="same")
        self.inception5a = InceptionModule(256, 160, 320, 32, 128, 128)
        self.inception5b = InceptionModule(384, 192, 384, 48, 128, 128)
        self.pool5 = tf.keras.layers.GlobalAveragePooling2D()
        self.dropout = tf.keras.layers.Dropout(0.4)
        self.fc = tf.keras.layers.Dense(10, activation="softmax")

    def call(self, x):
        x = tf.nn.relu(self.bn1(self.conv1(x)))
        x = self.pool1(x)
        x = tf.nn.relu(self.bn2(self.conv2(x)))
        x = self.pool2(x)
        x = self.inception3a(x)
        x = self.inception3b(x)
        x = self.pool3(x)
        x = self.inception4a(x)
        x = self.inception4b(x)
        x = self.inception4c(x)
        x = self.inception4d(x)
        x = self.inception4e(x)
        x = self.pool4(x)
        x = self.inception5a(x)
        x = self.inception5b(x)
        x = self.pool5(x)
        x = self.dropout(x)
        return self.fc(x)
