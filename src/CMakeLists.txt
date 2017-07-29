project(lmonit)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses)

if (CURSES_FOUND)
    add_executable(${PROJECT_NAME} info.c monit.c)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES} panel)
else()
    message(FATAL_ERROR "no curses found")
endif()
