import os
import cv2
import numpy as np
from PIL import Image
from extra import *
from extra import keypoints_and_edges_for_display
from model import *
from estimator import estimate



cam = cv2.VideoCapture(0)
template = 'template.jpg'


# Get the default frame width and height
frame_width = int(cam.get(cv2.CAP_PROP_FRAME_WIDTH))
frame_height = int(cam.get(cv2.CAP_PROP_FRAME_HEIGHT))

# Define the codec and create VideoWriter object
etalon, edges, colors = keypoints_and_edges_for_display(get_keypoints(template), frame_width, frame_height)

while True:
    ret, frame = cam.read()

    cv2.imwrite('camera-detect.jpg', frame)
    #hash = imagehash.average_hash(Image.open('camera-detect.jpg'))
    #gave frame
    keypoints = get_keypoints('camera-detect.jpg')
    locs, edges, colors = keypoints_and_edges_for_display(keypoints, frame_width, frame_height)
    #i know, where locs
    with_skeleton = pic_to_skeleton('camera-detect.jpg')[1]
    with_skeleton = cv2.resize(with_skeleton, (0,0), fx=0.25, fy=0.25)
    temp_skeleton = cv2.resize(pic_to_skeleton(template)[1], (0, 0), fx=0.1, fy=0.1)
    if estimate(etalon, locs):
        print('YES')

    else:
        print('NO')
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