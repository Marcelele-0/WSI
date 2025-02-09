import tensorflow as tf
from models.lenet import LeNet
from data.mnist import DataLoader

physical_devices = tf.config.list_physical_devices('GPU')
if physical_devices:
    try:
        tf.config.experimental.set_memory_growth(physical_devices[0], True)
        print("Using GPU")
    except:
        print("Failed to set memory growth for GPU")
else:
    print("Using CPU")

def compute_accuracy(model, dataset):
    """Oblicza dokładność modelu na podanym zbiorze"""
    correct = 0
    total = 0
    for images, labels in dataset:
        predictions = model(images)
        predicted_labels = tf.argmax(predictions, axis=1)
        labels = tf.cast(labels, tf.int64)  # Cast labels to int64
        correct += tf.reduce_sum(tf.cast(predicted_labels == labels, tf.int32)).numpy()
        total += labels.shape[0]
    return correct / total

def train_model(epochs=5, batch_size=32):
    model = LeNet(num_classes=10)
    model.build((None, 28, 28, 1))  # Inicjalizacja modelu

    optimizer = tf.optimizers.Adam()
    loss_fn = tf.losses.SparseCategoricalCrossentropy()

    # Ładowanie danych
    data_loader = DataLoader(batch_size=batch_size, data_dir="data")
    train_dataset = data_loader.get_dataset(train=True)
    test_dataset = data_loader.get_dataset(train=False)

    for epoch in range(epochs):
        print(f"Epoch {epoch+1}/{epochs}")
        epoch_loss = 0.0
        num_batches = 0

        for images, labels in train_dataset:
            with tf.GradientTape() as tape:
                predictions = model(images)
                loss = tf.reduce_mean(loss_fn(labels, predictions))
            
            gradients = tape.gradient(loss, model.trainable_variables)
            optimizer.apply_gradients(zip(gradients, model.trainable_variables))

            epoch_loss += loss.numpy()
            num_batches += 1

        avg_loss = epoch_loss / num_batches
        accuracy = compute_accuracy(model, test_dataset)
        
        print(f"Epoch {epoch+1}/{epochs} - Loss: {avg_loss:.4f}, Accuracy: {accuracy:.4%}")

if __name__ == "__main__":
    train_model(epochs=10)
