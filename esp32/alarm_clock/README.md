# 1. Set target

```Bash
idf.py set-target esp32s3
```


# 2. Add dependencies
```Bash
idf.py add-dependency "espressif/esp32-camera"
```


# 3. Configure project

```Bash
idf.py menuconfig
```

## set:
1. CONFIG_EXAMPLE_WIFI_SSID
2. CONFIG_EXAMPLE_WIFI_PASSWORD

## enable:
1. CONFIG_COMPILER_CXX_EXCEPTIONS
2. CONFIG_COMPILER_CXX_RTTI
3. PSRAM OCTO MOD & 80 MHz freq


# 4. Build application

```Bash
idf.py build
```


# 5. Flash chip

```Bash
idf.py flash
```


# 6. Check chip logs

```Bash
idf.py monitor
```
