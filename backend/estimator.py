import numpy as np
from math import sqrt
import os
_LIMIT = 30
def estimate(pattern, detect):
    mean = 0
    for i in detect:
        detect_points = detect[i][1] - detect[i][0]
        try:
            pattern_points = pattern[i][1] - pattern[i][0]
        except Exception:
            print(f'cannot get data about edge: {i}')
            continue
        #edges состоит из точки начала и конца

        dot = np.dot(detect_points, pattern_points) #скалярное умножение
        print(f'DOT: {dot}')
        detect_norm = np.linalg.norm(detect_points, ord=None) # норма Фробениуса
        pattern_norm = np.linalg.norm(pattern_points, ord=None)
        angle = np.degrees(np.arccos(dot/(detect_norm*pattern_norm)))
        print(f'ANGLE: {angle}')
        if angle > _LIMIT:
            print(f'NO. fix your {i}')
            return False

            # return False
    print('YES')
    return True