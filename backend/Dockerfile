FROM ubuntu:24.04 as build

WORKDIR /project/

RUN apt update
RUN apt install -y curl
RUN curl -LsSf https://astral.sh/uv/install.sh | sh

COPY pyproject.toml /project/
COPY .python-version /project/
RUN $HOME/.local/bin/uv sync

FROM build

COPY main.py /project/

CMD $HOME/.local/bin/uv run main.py
