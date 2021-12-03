cmake_minimum_required(VERSION 3.9)

set(EXEC_NAME Puzabrot)
project(${EXEC_NAME} VERSION 1.2 DESCRIPTION "Escape-time fractals viewer")

set(CMAKE_CXX_CLANG_TIDY
    clang-tidy;
    --format-style=file;
    --header-filter=*.h*;
)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.h*)

add_executable(${EXEC_NAME} ${SOURCES})
target_include_directories(${EXEC_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets)
file(COPY ${RESOURCE_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(${EXEC_NAME} PROPERTIES
    CXX_STANDARD          11
    CXX_STANDARD_REQUIRED ON
)

if(ADD_SANITIZERS)
    target_compile_options(${EXEC_NAME} PUBLIC -fsanitize=address -g)
    set_target_properties(${EXEC_NAME} PROPERTIES LINK_FLAGS "-fsanitize=address")
endif()

find_package(SFML 2.5.1 REQUIRED COMPONENTS graphics window system audio)

include_directories(${SFML_INCLUDE_DIR})
target_link_libraries(${EXEC_NAME} PRIVATE sfml-graphics sfml-window sfml-system sfml-audio)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -Wpedantic -Wcast-qual -Wcast-align -Wconversion \
    -Wsign-promo -Wfloat-equal -Wenum-compare -Wold-style-cast -Wredundant-decls -Wsign-conversion -Wnon-virtual-dtor \
    -Wctor-dtor-privacy -Woverloaded-virtual -Wno-float-equal -O3"
)