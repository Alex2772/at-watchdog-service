# Standard routine
cmake_minimum_required(VERSION 3.16)
project(at-watchdog)

set(AUI_VERSION bb77ba98)
option(BUILD_SHARED_LIBS OFF)

# Use AUI.Boot
file(
        DOWNLOAD
        https://raw.githubusercontent.com/aui-framework/aui/${AUI_VERSION}/aui.boot.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)

# import AUI
auib_import(aui https://github.com/aui-framework/aui
            COMPONENTS core json curl crypt image
            VERSION ${AUI_VERSION})


auib_import(date https://github.com/HowardHinnant/date)

set(TELEGRAM_API_TOKEN "" CACHE STRING "Telegram API token")

if (NOT TELEGRAM_API_TOKEN)
    message(FATAL_ERROR "TELEGRAM_API_TOKEN is not set. Please set it uo: -DTELEGRAM_API_TOKEN=BOTNAME:token")
endif()

aui_executable(at-watchdog-service)
target_compile_definitions(at-watchdog-service PRIVATE TELEGRAM_API_TOKEN=${TELEGRAM_API_TOKEN})

# Link required libs
aui_link(at-watchdog-service PUBLIC aui::core aui::json aui::curl aui::crypt aui::image)
