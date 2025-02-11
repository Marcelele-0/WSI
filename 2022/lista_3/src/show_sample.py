import os
import random

import hydra
import matplotlib.pyplot as plt
import numpy as np
import tensorflow as tf
from hydra.utils import instantiate
from omegaconf import DictConfig
from PIL import Image, ImageEnhance, ImageOps


def resize_and_pad(img, target_size=(28, 28), fill_color=0):
    """Resizes the image while maintaining aspect ratio and adds padding to achieve the target
    size.

    Args:
        img (PIL.Image): Input grayscale image.
        target_size (tuple): Target size (width, height). Default is (28, 28).
        fill_color (int): Fill color. Default is 0 (black).

    Returns:
        PIL.Image: Resized and centered image.
    """
    old_size = img.size  # (width, height)
    ratio = min(target_size[0] / old_size[0], target_size[1] / old_size[1])
    new_size = (int(old_size[0] * ratio), int(old_size[1] * ratio))
    img = img.resize(new_size, Image.LANCZOS)

    # Create new background with the specified size (default is black)
    new_img = Image.new("L", target_size, fill_color)
    paste_position = ((target_size[0] - new_size[0]) // 2, (target_size[1] - new_size[1]) // 2)
    new_img.paste(img, paste_position)
    return new_img


def load_images_from_folder(folder, sample_size):
    """Loads a specified number of images from a folder, transforms, and normalizes them.

    For each image:
      - Converts to grayscale,
      - Inverts colors to get white digits on a dark background,
      - Resizes the image while maintaining aspect ratio and adds padding to 28x28,
      - Normalizes the image (range [0, 1]).

    Args:
        folder (str): Path to the folder containing images (.jpg, .png, etc.).
        sample_size (int): Number of images to load.

    Returns:
        tuple: (images, filenames) where images is a numpy array of shape (sample_size, 28, 28, 1)
    """
    images = []
    filenames = random.sample(os.listdir(folder), sample_size)

    for filename in filenames:
        img_path = os.path.join(folder, filename)
        img = Image.open(img_path).convert("L")

        # Invert colors
        img = ImageOps.invert(img)

        # Enhance contrast
        enhancer = ImageEnhance.Contrast(img)
        img = enhancer.enhance(2.5)  # Increase contrast

        img = img.resize((28, 28))
        img = np.array(img).astype("float32") / 255.0  # Normalize to range [0, 1]
        images.append(img)

    images = np.array(images)  # Convert to numpy array after appending all images
    # Add channel dimension to get shape (sample_size, 28, 28, 1)
    return np.expand_dims(images, axis=-1), filenames


@hydra.main(version_base=None, config_path="../configs", config_name="train")
def show_sample(cfg: DictConfig):
    """Loads a saved model and displays predictions on randomly selected images of handwritten
    digits from the `handwritten` folder.

    Args:
        cfg (DictConfig): Hydra configuration.
    """
    sample_size = 4
    # Use os.path.join to correctly join paths
    data_dir = os.path.join(cfg.paths.data_dir, "handwritten")
    model_save_dir = cfg.paths.model_save_dir
    model_name = cfg.model._target_.split(".")[-1]
    model_save_path = os.path.join(model_save_dir, model_name)

    print(f"Loading model from {model_save_path}")

    try:
        model = tf.keras.models.load_model(model_save_path)
        model.summary()  # Display model architecture
        print(f"Model loaded from {model_save_path}")
    except Exception as e:
        print(f"Error loading model from {model_save_path}: {e}")
        return

    # Load images
    sample_images, filenames = load_images_from_folder(data_dir, sample_size)
    print(f"Sample images shape: {sample_images.shape}")

    # Predict on loaded images
    predictions = model.predict(sample_images)
    predicted_labels = np.argmax(predictions, axis=1)
    print(f"Predictions: {predictions}")
    print(f"Predicted labels: {predicted_labels}")

    # Display images and prediction results
    plt.figure(figsize=(10, 5))
    for i in range(sample_size):
        plt.subplot(1, sample_size, i + 1)
        plt.imshow(sample_images[i].reshape(28, 28), cmap="gray")
        plt.title(f"File: {filenames[i]}\nPred: {predicted_labels[i]}")
        plt.axis("off")
    plt.show()


if __name__ == "__main__":
    show_sample()
