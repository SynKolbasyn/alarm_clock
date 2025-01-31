from fastapi import FastAPI, Request
from starlette import status
from starlette.responses import Response
import numpy as np
import cv2
import uvicorn
from CV.extra import keypoints_and_edges_for_display
from CV.model import get_keypoints
from estimator import estimate

app = FastAPI()

template = 'template.jpg'
frame_width = 1280
frame_height = 720
etalon_keypoints, etalon_edges, colors, etalon_edges_with_names = keypoints_and_edges_for_display(get_keypoints(template), frame_width, frame_height, names=True)


@app.post("/")
async def root(request: Request):
  data: bytes = await request.body()
  print(len(data))
  image = cv2.imdecode(np.asarray(bytearray(data), dtype=np.uint8), cv2.IMREAD_COLOR)
  # cv2.imwrite('image.jpeg', image)

  keypoints, edges, edge_colors, edges_with_names = keypoints_and_edges_for_display(get_keypoints(image), names=True)

  is_correct_pose = estimate(etalon_edges_with_names, edges_with_names)
  return Response(status_code=status.HTTP_200_OK)