import os
import cv2
import numpy as np
from PIL import Image
from CV.extra import *
from CV.model import *
from CV.estimator import estimate



cam = cv2.VideoCapture(0)
template = 'template.jpg'


# Get the default frame width and height
frame_width = int(cam.get(cv2.CAP_PROP_FRAME_WIDTH))
frame_height = int(cam.get(cv2.CAP_PROP_FRAME_HEIGHT))

# Define the codec and create VideoWriter object
etalon_keypoints, etalon_edges, colors, etalon_edges_with_names = keypoints_and_edges_for_display(get_keypoints(template), frame_width, frame_height, names=True)

while True:
    ret, frame = cam.read()

    #gave frame
    # keypoints = get_keypoints('camera-detect.jpg')
    locs, edges, colors, edges_with_names = keypoints_and_edges_for_display(get_keypoints(frame, from_nparray=True), frame_width, frame_height, names=True)
    #i know, where locs
    with_skeleton = pic_to_skeleton(frame, from_nparray=True)
    with_skeleton = cv2.resize(with_skeleton, (0,0), fx=0.25, fy=0.25)
    temp_skeleton = cv2.resize(pic_to_skeleton(template), (0, 0), fx=0.3, fy=0.3)
    if estimate(etalon_edges_with_names, edges_with_names):
        pass
        # print('YES')
    else:
        # print('NO')
        pass
        # print('take correct pose')
    # Write the frame to the output file
    cv2.imshow('Skeleton', with_skeleton)
    cv2.imshow('Template', temp_skeleton)
    # Press 'q' to exit the loop
    if cv2.waitKey(1) == ord('q'):
        break


# Release the capture and writer objects
cam.release()
cv2.destroyAllWindows()
