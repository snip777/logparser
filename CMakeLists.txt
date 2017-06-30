cmake_minimum_required(VERSION 3.1)
project("logparser")

# for PVS studio 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall -O0 -ggdb -pg")

set(SRC main.cpp)

set(TARGET ${PROJECT_NAME})
add_executable(${TARGET} ${SRC})

add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND mkdir ARGS -p symbols/${TARGET}/`dump_syms -i ./${TARGET} | head -n1 | cut -d' ' -f4`
    COMMAND dump_syms ARGS ${TARGET} > symbols/${TARGET}/`dump_syms -i ${TARGET} | head -n1 | cut -d' ' -f4`/${TARGET}.sym
)

add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND pvs-studio-analyzer ARGS analyze -o ${TARGET}.log
    COMMAND plog-converter ARGS -a GA:1,2 -t tasklist ${TARGET}.log
)

