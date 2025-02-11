import hydra
import numpy as np
import tensorflow as tf
from data.mnist import DataLoader
from hydra.utils import instantiate
from metrics.accuracy import compute_accuracy
from omegaconf import DictConfig
from sklearn.utils import class_weight


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

    # Compute class weights
    y_train = np.concatenate([y for _, y in train_dataset], axis=0)
    class_weights = class_weight.compute_class_weight(
        "balanced", classes=np.unique(y_train), y=y_train
    )
    class_weights_dict = {i: class_weights[i] for i in range(len(class_weights))}

    model.compile(optimizer=optimizer, loss=loss_fn, metrics=["accuracy"])

    model.fit(
        train_dataset,
        epochs=num_of_epochs,
        validation_data=test_dataset,
        class_weight=class_weights_dict,
    )

    train_accuracy = compute_accuracy(model, train_dataset)
    test_accuracy = compute_accuracy(model, test_dataset)

    print(f"Final - Train Accuracy: {train_accuracy:.4%}, Test Accuracy: {test_accuracy:.4%}")

    model_save_path = f"{model_save_dir}/{model_name}"
    print(f"Saving model to {model_save_path}")  # Debug print
    model.save(model_save_path)


if __name__ == "__main__":
    train_model()
