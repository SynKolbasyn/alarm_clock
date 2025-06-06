import cv2
import asyncio
import base64
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import HTMLResponse
import uvicorn
from typing import Optional
from CV.model import *
from CV.estimator import estimate
from http import HTTPStatus
from typing import Annotated
from fastapi import FastAPI, Request, Depends, Response
import numpy as np
import cv2
from CV.extra import keypoints_and_edges_for_display
from CV.model import get_keypoints
from CV.estimator import estimate

app = FastAPI()

# Глобальная переменная для хранения кадра с камеры
current_frame: Optional[bytes] = None
is_correct_pose: Optional[bool] = None
# HTML страница для отображения видео
html = """
<!DOCTYPE html>
<html>
<head>
    <title>WebCam Stream</title>
    <style>
        body { margin: 0; padding: 20px; text-align: center; }
        #video { max-width: 100%; max-height: 80vh; border: 1px solid #ccc; }
        .controls { margin: 20px 0; }
    </style>
</head>
<body>
    <h1>WebCam Live Stream</h1>
    <img id="video" src="" alt="Video Stream">
    <p id="text"></p>
    
    <script>
        const video = document.getElementById('video');
        const text = document.getElementById('text');
        const ws = new WebSocket(`ws://${window.location.host}/ws`);
        const ws_text = new WebSocket(`ws://${window.location.host}/ws_text`);
        
        ws_text.onmessage = function(event) {
            if (event.data) {
                text.innerHTML = event.data;
            }
        };

        // Обработчик получения нового кадра
        ws.onmessage = function(event) {
            if (event.data) {
                video.src = 'data:image/jpeg;base64,' + event.data;
            }
        };

        // Обработчик ошибок
        ws.onerror = function(error) {
            console.error('WebSocket error:', error);
        };
    </script>
</body>
</html>
"""


#load pattern image
_, _, _, etalon_edges_with_names = keypoints_and_edges_for_display(get_keypoints('template.jpg'),
                                                                   1280, 720, names=True)


async def capture_frames():
    """Функция для захвата кадров с веб-камеры"""
    global current_frame, is_correct_pose
    cap = cv2.VideoCapture(0)

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                break

            # Изменяем размер и конвертируем в JPEG
            frame = cv2.resize(frame, (640, 480))
            frame = cv2.rotate(frame, cv2.ROTATE_180)
            frame= pic_to_skeleton(frame, from_nparray=True)
            frame = cv2.resize(frame, (0, 0), fx=0.25, fy=0.25)
            _, buffer = cv2.imencode('.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, 70])

            _, _, _, edges_with_names = keypoints_and_edges_for_display(get_keypoints(frame, from_nparray=True), 1280,
                                                                        720, names=True)
            is_correct_pose = estimate(etalon_edges_with_names, edges_with_names)
            current_frame = buffer.tobytes()
            await asyncio.sleep(0.1)  # ~30 FPS
    finally:
        cap.release()


@app.get("/stream/")
async def get():
    return HTMLResponse(html)


@app.post("/")
async def root(request: Request, etalon_edges_with_names: Annotated[dict, Depends(get_etalon_edges_with_names)]):
    data: bytes = await request.body()
    print(len(data))
    image = cv2.imdecode(np.asarray(bytearray(data), dtype=np.uint8), cv2.IMREAD_COLOR)
    image = cv2.rotate(image, cv2.ROTATE_180)

    _, _, _, edges_with_names = keypoints_and_edges_for_display(get_keypoints(image, from_nparray=True), 1280, 720,
                                                                names=True)

    is_correct_pose = estimate(etalon_edges_with_names, edges_with_names)
    response = HTTPStatus.OK if is_correct_pose else HTTPStatus.BAD_REQUEST
    return Response(status_code=response)


@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    """WebSocket endpoint для трансляции видео"""
    await websocket.accept()
    try:
        while True:
            if current_frame:
                # Кодируем кадр в base64
                frame_base64 = base64.b64encode(current_frame).decode('utf-8')
                await websocket.send_text(frame_base64)
            await asyncio.sleep(0.033)  # ~30 FPS
    except WebSocketDisconnect:
        print("Client disconnected")


@app.websocket("/ws_text")
async def websocket_endpoint_text(websocket: WebSocket):
    """WebSocket endpoint для трансляции видео"""
    await websocket.accept()
    try:
        while True:
            if is_correct_pose:
                # Кодируем кадр в base64
                await websocket.send_text("Ок")
            else:
                await websocket.send_text("нот Ок")
            await asyncio.sleep(0.033)  # ~30 FPS
    except WebSocketDisconnect:
        print("Client disconnected")


@app.on_event("startup")
async def startup_event():
    """Запускаем захват кадров при старте приложения"""
    asyncio.create_task(capture_frames())


if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)