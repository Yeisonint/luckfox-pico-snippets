cmake_minimum_required(VERSION 3.10)

set(SDK_PATH $ENV{LUCKFOX_SDK_PATH})

if (NOT SDK_PATH)
    message(FATAL_ERROR "Please set LUCKFOX_SDK_PATH. Example:
export LUCKFOX_SDK_PATH=/home/user/luckfox-pico")
endif()

set(CMAKE_C_COMPILER "${SDK_PATH}/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-gcc")

project(c_example C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.c
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wshadow
    -Wformat=2
    -O2
)

target_link_libraries(${PROJECT_NAME}
    Threads::Threads
)