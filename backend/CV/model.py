import os
import tensorflow as tf
from matplotlib import pyplot as plt
from matplotlib.collections import LineCollection
import matplotlib.patches as patches
import imageio
from IPython.display import HTML, display
from .extra import *

path = os.getcwd()
print(path)
model_name = "movenet_lightning"
input_size = 256
interpreter = tf.lite.Interpreter(model_path=f"{path}/CV/model.tflite")
interpreter.allocate_tensors()


def movenet(input_image):
    """
    Runs pose estimation on the input image using the MoveNet TensorFlow Lite model.

    Args:
        input_image (tf.Tensor): Input image tensor of shape (1, height, width, 3)
                                with dtype tf.uint8

    Returns:
        numpy.ndarray: Array of keypoints with scores of shape (1, 1, 17, 3) where:
                       - First two values in last dimension are (y, x) coordinates (normalized to 0-1)
                       - Third value is the confidence score (0-1)
    """
    input_image = tf.cast(input_image, dtype=tf.uint8)
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()
    interpreter.set_tensor(input_details[0]['index'], input_image.numpy())
    interpreter.invoke()
    keypoints_with_scores = interpreter.get_tensor(output_details[0]['index'])
    return keypoints_with_scores


def pic_to_skeleton(image, from_nparray=False):
    """
    Processes an image to detect human pose and returns an image with skeleton overlay.

    Args:
        image (str or numpy.ndarray): Either a path to an image file or a numpy array
                                     containing image data
        from_nparray (bool): If True, treats 'image' as a numpy array instead of file path

    Returns:
        numpy.ndarray: Output image with skeleton overlay drawn on it, shape (height, width, 3)
    """
    if from_nparray:
        image = tf.convert_to_tensor(image, dtype=np.uint8)
        pass
    else:
        image_path = image
        image = tf.io.read_file(image_path)
        image = tf.convert_to_tensor(image)
        image = tf.image.decode_jpeg(image)

    input_image = tf.expand_dims(image, axis=0)
    input_image = tf.image.resize_with_pad(input_image, input_size, input_size)
    keypoints_with_scores = movenet(input_image)

    display_image = tf.expand_dims(image, axis=0)
    display_image = tf.cast(tf.image.resize_with_pad(
        display_image, 1280, 1440), dtype=tf.int32)
    output_overlay, edges_with_names = draw_prediction_on_image(
        np.squeeze(display_image.numpy(), axis=0), keypoints_with_scores)

    return output_overlay


def get_keypoints(image, from_nparray=False):
    """
    Extracts human pose keypoints from an image using MoveNet model.

    Args:
        image (str or numpy.ndarray): Either a path to an image file or a numpy array
                                     containing image data
        from_nparray (bool): If True, treats 'image' as a numpy array instead of file path

    Returns:
        numpy.ndarray: Array of keypoints with scores of shape (1, 1, 17, 3) where:
                       - First two values in last dimension are (y, x) coordinates (normalized to 0-1)
                       - Third value is the confidence score (0-1)
    """
    if from_nparray:
        image = tf.convert_to_tensor(image, dtype=np.uint8)
        pass
    else:
        image_path = image
        image = tf.io.read_file(image_path)
        image = tf.convert_to_tensor(image)
        image = tf.image.decode_jpeg(image)

    input_image = tf.expand_dims(image, axis=0)
    input_image = tf.image.resize_with_pad(input_image, input_size, input_size)
    keypoints_with_scores = movenet(input_image)
    return keypoints_with_scores