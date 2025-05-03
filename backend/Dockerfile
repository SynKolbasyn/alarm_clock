FROM ubuntu:24.04 as build

WORKDIR /project/

RUN apt-get update
RUN apt-get install -y curl
RUN apt-get install ffmpeg libsm6 libgl1 libxext6 -y
RUN curl -LsSf https://astral.sh/uv/install.sh | sh

COPY . /project/
COPY .python-version /project/
RUN $HOME/.local/bin/uv sync

FROM build



CMD $HOME/.local/bin/uv run gunicorn server:app --workers $(nproc) --worker-class uvicorn.workers.UvicornWorker --bind 0.0.0.0:80