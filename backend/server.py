from http import HTTPStatus
from typing import Annotated
from fastapi import FastAPI, Request, Depends, Response
import numpy as np
import cv2
from CV.extra import keypoints_and_edges_for_display
from CV.model import get_keypoints
from CV.estimator import estimate

app = FastAPI()


async def get_etalon_edges_with_names():
    _, _, _, res = keypoints_and_edges_for_display(get_keypoints('template.jpg'), 1280, 720, names=True)
    return res



@app.post("/")
async def root(request: Request, etalon_edges_with_names: Annotated[dict, Depends(get_etalon_edges_with_names)]):
    data: bytes = await request.body()
    print(len(data))
    image = cv2.imdecode(np.asarray(bytearray(data), dtype=np.uint8), cv2.IMREAD_COLOR)
    
    _, _, _, edges_with_names = keypoints_and_edges_for_display(get_keypoints(image), names=True)

    is_correct_pose = estimate(etalon_edges_with_names, edges_with_names)
    response = HTTPStatus.OK if is_correct_pose else HTTPStatus.BAD_REQUEST
    return Response(status_code=response)