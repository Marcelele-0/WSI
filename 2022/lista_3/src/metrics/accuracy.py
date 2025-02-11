import tensorflow as tf


def compute_accuracy(model, dataset):
    """Compute the accuracy of the model on the dataset.

    Args:
    - model: A tf.keras.Model object
    - dataset: A tf.data.Dataset object
    """
    correct = 0
    total = 0
    for images, labels in dataset:
        predictions = model(images)
        predicted_labels = tf.argmax(predictions, axis=1)
        labels = tf.cast(labels, tf.int64)  # Cast labels to int64
        correct += tf.reduce_sum(tf.cast(predicted_labels == labels, tf.int32)).numpy()
        total += labels.shape[0]
    return correct / total
