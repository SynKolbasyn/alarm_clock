# 1. Set target

```Bash
idf.py set-target <esp32>
```


# 2. Configure project

```Bash
idf.py menuconfig
```

## set:
1. CONFIG_EXAMPLE_WIFI_SSID
2. CONFIG_EXAMPLE_WIFI_PASSWORD

## enable:
1. CONFIG_COMPILER_CXX_EXCEPTIONS
2. CONFIG_COMPILER_CXX_RTTI


# 3. Build application

```Bash
idf.py build
```


# 4. Flash chip

```Bash
idf.py flash
```


# 5. Check chip logs

```Bash
idf.py monitor
```
