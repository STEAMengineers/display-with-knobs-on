add_library( screen
    screen.h
    screen.c
    )

target_link_libraries(screen
    hardware_spi
    pico_stdlib
    lvgl
)

target_include_directories(screen PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")