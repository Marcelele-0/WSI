import tensorflow as tf

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
