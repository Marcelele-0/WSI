import tensorflow as tf
from models.lenet import LeNet
from data.mnist import DataLoader

def train_model():
    model = LeNet(num_classes=10)
    model(tf.keras.Input(shape=(28, 28, 1)))  # Przygotowanie modelu do treningu

    # Tworzenie optymalizatora
    optimizer = tf.optimizers.Adam()

    # Przygotowanie danych
    data_loader = DataLoader(batch_size=32)
    for images, labels in data_loader:
        with tf.GradientTape() as tape:
            predictions = model(images)
            loss = tf.reduce_mean(tf.losses.sparse_categorical_crossentropy(labels, predictions))
        gradients = tape.gradient(loss, model.trainable_variables)
        optimizer.apply_gradients(zip(gradients, model.trainable_variables))
        print(f"Loss: {loss.numpy()}")

if __name__ == "__main__":
    train_model()
