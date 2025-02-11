import hydra
import tensorflow as tf
from data.mnist import DataLoader
from hydra.utils import instantiate
from metrics.accuracy import compute_accuracy
from omegaconf import DictConfig


@hydra.main(version_base=None, config_path="../configs", config_name="train")
def train_model(cfg: DictConfig):
    """Train the model based on the provided configuration.

    Args:
        cfg (DictConfig): Configuration composed by Hydra.
    """
    num_of_epochs = cfg.trainer.epochs
    batch_size = cfg.trainer.batch_size
    learning_rate = cfg.optimizer.lr
    data_dir = cfg.paths.data_dir
    model_save_dir = cfg.paths.model_save_dir
    model_name = cfg.model._target_.split(".")[-1]  # Extract the actual model name
    model = instantiate(cfg.model)
    model.build((None, 28, 28, 1))

    optimizer = tf.optimizers.Adam(learning_rate)
    loss_fn = tf.losses.SparseCategoricalCrossentropy()

    data_loader = DataLoader(batch_size, data_dir)
    train_dataset = data_loader.get_dataset(train=True)
    test_dataset = data_loader.get_dataset(train=False)

    for epoch in range(num_of_epochs):
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
        train_accuracy = compute_accuracy(model, train_dataset)
        test_accuracy = compute_accuracy(model, test_dataset)

        print(
            f"Epoch {epoch + 1}/{num_of_epochs} - Loss: {avg_loss:.4f}, "
            f"Train Accuracy: {train_accuracy:.4%}, Test Accuracy: {test_accuracy:.4%}"
        )

    model_save_path = f"{model_save_dir}/{model_name}"
    model.save(model_save_path)


if __name__ == "__main__":
    train_model()
