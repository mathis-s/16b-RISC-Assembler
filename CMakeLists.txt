cmake_minimum_required(VERSION 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "-Wall -Wextra")
project(Compiler)

add_executable(asm
src/Disassembler.c
src/GenericList.c
src/Lexer.c
src/Main.c
src/Util.c
)
set_property(TARGET asm PROPERTY C_STANDARD 11)