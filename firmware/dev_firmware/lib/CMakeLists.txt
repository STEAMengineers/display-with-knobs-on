add_library( screen
    screen.c
    motors.c
    )
target_include_directories(screen PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(screen
    hardware_spi
    hardware_pwm
    hardware_gpio
    pico_stdlib
    lvgl
)

