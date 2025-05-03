import numpy as np
from math import sqrt
import os
_LIMIT = 30
def estimate(pattern, detect):
    mean = 0
    for i in pattern:
        if i not in detect:
            print(f'cannot get data about {i}')
            continue
        detect_points = detect[i][1] - detect[i][0]
        pattern_points = pattern[i][1] - pattern[i][0]

        #edges состоит из точки начала и конца

        dot = np.dot(detect_points, pattern_points) #скалярное умножение
        detect_norm = np.linalg.norm(detect_points, ord=None) # норма Фробениуса
        pattern_norm = np.linalg.norm(pattern_points, ord=None)
        angle = np.degrees(np.arccos(dot/(detect_norm*pattern_norm)))
        if angle > _LIMIT:
            os.system('clear')
            print(f'ANGLE: {angle}\n'
                  f'NO. fix your {i}', flush=True)

            return False

    print('YES')
    return True