FROM ubuntu:24.04

WORKDIR /project/

RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install -y curl
RUN apt-get install -y ffmpeg
RUN apt-get install -y libsm6
RUN apt-get install -y libgl1
RUN apt-get install -y libxext6
RUN curl -LsSf https://astral.sh/uv/install.sh | sh

COPY ./.python-version ./
RUN $HOME/.local/bin/uv python install

COPY ./pyproject.toml ./
RUN $HOME/.local/bin/uv sync

COPY ./template.jpg ./
COPY ./server.py ./
COPY ./CV/ ./CV/

CMD $HOME/.local/bin/uv run gunicorn server:app --workers $(nproc) --worker-class uvicorn.workers.UvicornWorker --bind 0.0.0.0:80
