import cv2, time
from model import *

while True:
    try:
        image = cv2.imread("image.jpeg")
        cv2.imshow("window", image)
        with_skeleton = pic_to_skeleton('image.jpeg')
        cv2.imshow('Window2', with_skeleton)

    except Exception:
        ...
    if cv2.waitKey(1) == ord('q'):
        break