import os
import random

import hydra
import matplotlib.pyplot as plt
import numpy as np
from hydra.utils import instantiate
from omegaconf import DictConfig
from PIL import Image


def load_images_from_folder(folder, sample_size):
    """Load sample_size images from the folder.

    Args:
    folder: str, path to the folder containing .jpg images
    sample_size: int, number of images to load
    """
    images = []
    filenames = random.sample(os.listdir(folder), sample_size)
    for filename in filenames:
        img = Image.open(os.path.join(folder, filename)).convert("L")
        img = img.resize((28, 28))
        img = np.array(img) / 255.0
        images.append(img)
    return np.expand_dims(np.array(images), axis=-1), filenames


@hydra.main(version_base=None, config_path="../configs", config_name="train")
def show_sample(cfg: DictConfig):
    """Load the model and show predictions on a sample of hand written images."""

    sample_size = 5
    data_dir = cfg.paths.data_dir + "handwritten/"
    model_save_dir = cfg.paths.model_save_dir
    model_name = cfg.model.name
    model_save_path = f"{model_save_dir}/{model_name}"

    model = instantiate(cfg.model)  # Ensure model is instantiated
    model.build((None, 28, 28, 1))
    model.load_weights(model_save_path)

    sample_images, filenames = load_images_from_folder(data_dir, sample_size)
    predictions = model.predict(sample_images)
    predicted_labels = np.argmax(predictions, axis=1)

    plt.figure(figsize=(10, 5))
    for i in range(sample_size):
        plt.subplot(1, sample_size, i + 1)
        plt.imshow(sample_images[i].reshape(28, 28), cmap="gray")
        plt.title(f"File: {filenames[i]}, Pred: {predicted_labels[i]}")
        plt.axis("off")
    plt.show()


if __name__ == "__main__":
    show_sample()
