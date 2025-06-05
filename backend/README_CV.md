Загрузиn в эту папку картинку с позой вместо ```template.jpg```

Для локального тестирования нужно установить зависимости.

```console
sudo apt update
apt upgrade -y
apt install -y curl
apt install -y ffmpeg
apt install -y libsm6
apt install -y libgl1
apt install -y libxext6
curl -LsSf https://astral.sh/uv/install.sh | sh

$HOME/.local/bin/uv python install
$HOME/.local/bin/uv sync
```

и запустить таким образом
```console
sudo $HOME/.local/bin/uv run cam-test.py
```
