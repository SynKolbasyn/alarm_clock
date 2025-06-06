import tensorflow as tf

import os

# Import matplotlib libraries
from matplotlib import pyplot as plt
from matplotlib.collections import LineCollection
import matplotlib.patches as patches

from .extra import *

model_name = "movenet_lightning"
input_size = 256
interpreter = tf.lite.Interpreter(model_path="./CV/model.tflite")
interpreter.allocate_tensors()

def movenet(input_image):
    input_image = tf.cast(input_image, dtype=tf.uint8)
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()
    interpreter.set_tensor(input_details[0]['index'], input_image.numpy())
    interpreter.invoke()
    keypoints_with_scores = interpreter.get_tensor(output_details[0]['index'])
    return keypoints_with_scores



def pic_to_skeleton(image, from_nparray=False):
    """transform picture to skeleton image. Function for debug.

    Args:
        image: a detected picture by web camera. Can be a nparray and path to picture/
        from_nparray: how parse image
    Returns:
        picture of skeleton"""
    if from_nparray:
        image = tf.convert_to_tensor(image, dtype=np.uint8)
        # image = tf.image.decode_jpeg(image)
        pass
    else:
        image_path = image
        image = tf.io.read_file(image_path)
        image = tf.convert_to_tensor(image)
        image = tf.image.decode_jpeg(image)

    input_image = tf.expand_dims(image, axis=0)

    input_image = tf.image.resize_with_pad(input_image, input_size, input_size)

    # Run model inference.
    keypoints_with_scores = movenet(input_image)

    # Visualize the predictions with image.
    display_image = tf.expand_dims(image, axis=0)
    display_image = tf.cast(tf.image.resize_with_pad(
        display_image, 1280, 1440), dtype=tf.int32)
    output_overlay, edges_with_names = draw_prediction_on_image(
        np.squeeze(display_image.numpy(), axis=0), keypoints_with_scores)

    #plt.figure(figsize=(5, 5))
    #plt.imsave(f'result.jpg', output_overlay)
    return output_overlay


def get_keypoints(image, from_nparray=False):
    """get keypoints on skeleton.
    Args:
        image: a detected picture by web camera. Can be a nparray and path to picture/
        from_nparray: how parse image
    Returns:
        keypoints"""
    if from_nparray:
        image = tf.convert_to_tensor(image, dtype=np.uint8)
    else:
        image_path = image
        image = tf.io.read_file(image_path)
        image = tf.convert_to_tensor(image)
        image = tf.image.decode_jpeg(image)

    input_image = tf.expand_dims(image, axis=0)
    input_image = tf.image.resize_with_pad(input_image, input_size, input_size)
    keypoints_with_scores = movenet(input_image)
    return keypoints_with_scores

