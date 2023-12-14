cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(main_project C CXX ASM)
set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

FILE(GLOB MyCSources Images/*.c)

add_executable(main
  main_board.c
  LivingBoardGameSupporting/Supporting/misc.c
  LivingBoardGameSupporting/pn532/PN532_SPI.c
  LivingBoardGameSupporting/pn532/PN532.c
  LivingBoardGameSupporting/SSD1306/SSD1306.c
  LivingBoardGameSupporting/game.c
  LivingBoardGameSupporting/Servo/servo.c
  ${MyCSources}
)

target_include_directories(main PRIVATE LivingBoardGameSupporting/Supporting/ LivingBoardGameSupporting/pn532/ ./ LivingBoardGameSupporting/SSD1306/ Images/ LivingBoardGameSupporting/ LivingBoardGameSupporting/Servo/)

pico_enable_stdio_usb(main 1)

pico_add_extra_outputs(main)

target_link_libraries(main pico_stdlib hardware_spi hardware_i2c hardware_pwm)