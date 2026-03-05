# Luckfox pico snippets
Useful snippets for Luckfox pico boards.

## Get the SDK
Download the SDK version used in this repository

```bash
git submodule update --init
```

## Prepare the board configuration

Go to the SDK folder and execute:

```bash
cd luckfox-pico
./build.sh lunch
```

Follow the steps depending on the board you will be using and whether you will be using NAND flash memory or an SD card. For example:

```bash
luckfox-pico-snippets $ cd luckfox-pico
luckfox-pico-snippets/luckfox-pico$ ./build.sh lunch
You're building on Linux
  Lunch menu...pick the Luckfox Pico hardware version:
  选择 Luckfox Pico 硬件版本:
                [0] RV1103_Luckfox_Pico
                [1] RV1103_Luckfox_Pico_Mini
                [2] RV1103_Luckfox_Pico_Plus
                [3] RV1103_Luckfox_Pico_WebBee
                [4] RV1106_Luckfox_Pico_Pro_Max
                [5] RV1106_Luckfox_Pico_Ultra
                [6] RV1106_Luckfox_Pico_Ultra_W
                [7] RV1106_Luckfox_Pico_Pi
                [8] RV1106_Luckfox_Pico_Pi_W
                [9] RV1106_Luckfox_Pico_86Panel
                [10] RV1106_Luckfox_Pico_86Panel_W
                [11] RV1106_Luckfox_Pico_Zero
                [12] custom
Which would you like? [0~12][default:0]: 1
  Lunch menu...pick the boot medium:
  选择启动媒介:
                [0] SD_CARD
                [1] SPI_NAND
Which would you like? [0~1][default:0]: 1
  Lunch menu...pick the system version:
  选择系统版本:
                [0] Buildroot 
Which would you like? [0][default:0]: 0
[build.sh:info] Lunching for Default BoardConfig_IPC/BoardConfig-SPI_NAND-Buildroot-RV1103_Luckfox_Pico_Mini-IPC.mk boards...
[build.sh:info] switching to board: luckfox-pico-snippets/luckfox-pico/project/cfg/BoardConfig_IPC/BoardConfig-SPI_NAND-Buildroot-RV1103_Luckfox_Pico_Mini-IPC.mk
[build.sh:info] Running build_select_board succeeded.
```

Finally, compile everything and you'll be ready to follow the individual steps for each snippet.

```bash
luckfox-pico-snippets/luckfox-pico$ ./build.sh
```