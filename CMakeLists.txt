
cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)
set(FREERTOS_KERNEL_PATH "/home/rodrigo-piropo/EMBARCA_TECH_2/FreeRTOS-Kernel")
set(FREERTOS_KERNEL_PATH "/home/rodrigo-piropo/EMBARCA_TECH_2/FreeRTOS-Kernel")
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(PiscaLed C CXX ASM)
pico_sdk_init()

include_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(${PROJECT_NAME}  
        blink.c 
        lib/ssd1306.c
        )

#target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/ws2818b.pio)

target_link_libraries(${PROJECT_NAME} 
        pico_stdlib
        pico_bootrom
        hardware_pwm
        hardware_pio
        hardware_i2c
        FreeRTOS-Kernel         # Kernel do FreeRTOS
        FreeRTOS-Kernel-Heap4   # Gerenciador de memoria
        )

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})




