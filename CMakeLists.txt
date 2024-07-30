cmake_minimum_required(VERSION 3.10)
project(lvgl)

set(CMAKE_C_STANDARD 99)#C99 # lvgl officially support C99 and above
set(CMAKE_CXX_STANDARD 17)#C17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# LVGL
add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR})

# CURL
find_package(CURL REQUIRED) 
include_directories(${CURL_INCLUDE_DIR})

# coffeeCalendar
add_executable(coffeeCalendar src/main.c src/error.c src/mouse_cursor_icon.c src/ical2ccal.c src/ical.c src/ccal.c src/calutils.c src/config.c)

target_include_directories(coffeeCalendar PRIVATE include)
target_link_libraries(coffeeCalendar lvgl lvgl::thorvg ${SDL2_LIBRARIES} m pthread ${CURL_LIBRARIES} ical)
add_custom_target (run COMMAND ${EXECUTABLE_OUTPUT_PATH}/coffeeCalendar DEPENDS coffeeCalednar)
