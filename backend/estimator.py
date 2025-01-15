import numpy as np
from math import sqrt
import os
_LIMIT = 100
def estimate(pattern, points):
    mean = 0
    for i in range(len(points)):
        try:
            x1, y1 = points[i]
            x2, y2 = pattern[i]
            dist = sqrt( (x2 - x1)**2 + (y2 - y1)**2)
            if dist > _LIMIT:
                return False
        except Exception:
            print('cannot get data about keypoint')
            return False

    return True