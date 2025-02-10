import tensorflow as tf
from data.mnist import DataLoader
from metrics.accuracy import compute_accuracy  # Import the compute_accuracy function
import hydra
from omegaconf import DictConfig
from hydra.utils import instantiate

@hydra.main(version_base=None, config_path="../configs", config_name="train")
def train_model(cfg: DictConfig):
    num_of_epochs = cfg.trainer.epochs
    batch_size = cfg.trainer.batch_size
    learning_rate = cfg.optimizer.lr
    data_dir = cfg.paths.data_dir
    model_save_dir = cfg.paths.model_save_dir

    model = instantiate(cfg.model)  # Instantiate the model using Hydra    
    model.build((None, 28, 28, 1))  # Inicjalizacja modelu

    optimizer = tf.optimizers.Adam(learning_rate)  # Use cfg.optimizer.lr
    loss_fn = tf.losses.SparseCategoricalCrossentropy()

    # Ładowanie danych
    data_loader = DataLoader(batch_size, data_dir)
    train_dataset = data_loader.get_dataset(train=True)
    test_dataset = data_loader.get_dataset(train=False)

    for epoch in range(num_of_epochs):
        print(f"Epoch {epoch+1}/{num_of_epochs}")
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
        
        print(f"Epoch {epoch+1}/{num_of_epochs} - Loss: {avg_loss:.4f}, Accuracy: {accuracy:.4%}")

    # Save the model
    model.save(model_save_dir)

if __name__ == "__main__":
    train_model()
