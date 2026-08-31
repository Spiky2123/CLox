cmake_minimum_required(VERSION 4.3)
project(CLox C)

set(CMAKE_C_STANDARD 11)

add_executable(CLox src/main.c
        src/common.h
        src/chunk.h
        src/chunk.c
        src/memory.h
        src/memory.c
        src/debug.h
        src/debug.c
        src/value.h
        src/value.c
        src/vm.h
        src/vm.c
        src/compiler.h
        src/compiler.c
        src/scanner.h
        src/scanner.c
        src/object.h
        src/object.c
        src/table.h
        src/table.c)
