cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME stmf7-folder)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Disable LVGL demos/examples/thorvg (not needed for embedded)
set(LV_CONF_BUILD_DISABLE_EXAMPLES ON CACHE BOOL "" FORCE)
set(LV_CONF_BUILD_DISABLE_DEMOS ON CACHE BOOL "" FORCE)
set(LV_CONF_BUILD_DISABLE_THORVG_INTERNAL ON CACHE BOOL "" FORCE)
set(LV_CONF_PATH "${CMAKE_SOURCE_DIR}/Core/Inc/lv_conf.h" CACHE STRING "" FORCE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Add LVGL library
add_subdirectory(Middlewares/Third_Party/lvgl)

# LVGL needs FreeRTOS and CMSIS headers when LV_USE_OS == LV_OS_FREERTOS
target_include_directories(lvgl SYSTEM PUBLIC
    Middlewares/Third_Party/FreeRTOS/Source/include
    Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1
    Core/Inc
    Drivers/CMSIS/Include
    Drivers/CMSIS/Device/ST/STM32F7xx/Include
    Drivers/STM32F7xx_HAL_Driver/Inc
)
target_compile_definitions(lvgl PUBLIC USE_HAL_DRIVER STM32F746xx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Src/sdram.c
    Core/Src/lv_port_disp.c
    Core/Src/lv_port_indev.c
    Core/Src/gamepad.c
    Core/Src/ui_screens.c
    Core/Src/images/img_haz1.c
    Core/Src/images/img_haz2.c
    Core/Src/images/img_lights_off.c
    Core/Src/images/img_lights_on.c
    Core/Src/images/img_lights_high.c
    Core/Src/images/img_abs_off.c
    Core/Src/images/img_abs_on.c
    Core/Src/images/img_esc_off.c
    Core/Src/images/img_esc_on.c
    Core/Src/images/img_pbrake_off.c
    Core/Src/images/img_pbrake_on.c
    Core/Src/images/img_range.c
    Core/Src/images/img_diff_open.c
    Core/Src/images/img_diff_locked.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    lvgl

    # Add user defined libraries
)
