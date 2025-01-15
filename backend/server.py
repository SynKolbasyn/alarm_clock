import time

import PIL.Image
from fastapi import FastAPI, Request
from starlette import status
from starlette.responses import Response
# from PIL import Image
from io import BytesIO
import numpy as np
import cv2
from threading import Thread

from estimator import estimate

app = FastAPI()


@app.post("/")
async def root(request: Request):
  data: bytes = await request.body()
  print(len(data))
  image = cv2.imdecode(np.asarray(bytearray(data), dtype=np.uint8), cv2.IMREAD_COLOR)
  cv2.imwrite('image.jpeg', image)
  # keypoints, image =
  return Response(status_code=status.HTTP_200_OK)
