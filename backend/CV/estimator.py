import numpy as np
from math import sqrt
import os

_LIMIT = 30


def estimate(pattern, detect):
    """compare detected image with pattern
    Args:
        pattern: pattern picture, etalon.
        detect: detected image by esp32.
    Returns:
        Flag - is detected image is similar with pattern."""
    mean = 0
    for i in detect:
        os.system('clear')
        detect_points = detect[i][1] - detect[i][0]
        try:
            pattern_points = pattern[i][1] - pattern[i][0]
        except Exception:
            print(f'cannot get data about edge: {i}')
            return False
        #edges состоит из точки начала и концаq
        dot = np.dot(detect_points, pattern_points) #скалярное умножение
        detect_norm = np.linalg.norm(detect_points, ord=None) # норма Фробениуса
        pattern_norm = np.linalg.norm(pattern_points, ord=None)
        angle = np.degrees(np.arccos(dot/(detect_norm*pattern_norm)))
        print(f'ANGLE {i}: {angle}')
        if angle > _LIMIT:
            print(f'NO. fix your {i}')
            return False

    print('YES')
    return True