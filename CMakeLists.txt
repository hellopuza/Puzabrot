cmake_minimum_required(VERSION 3.9)

set(EXEC_NAME Puzabrot)
project(${EXEC_NAME} VERSION 1.2 DESCRIPTION "Escape-time fractals viewer")

file(GLOB_RECURSE ALL_CXX_SOURCE_FILES *.cpp *.h*)
find_program(CLANG_FORMAT "clang-format")
add_custom_target(
    format
    COMMAND clang-format-12
    -i
    -style=file
    ${ALL_CXX_SOURCE_FILES}
)

set(CMAKE_CXX_CLANG_TIDY
    clang-tidy;
    --format-style=file;
    --header-filter=*.h*;
)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.h*)

add_executable(${EXEC_NAME} ${SOURCES})
target_include_directories(${EXEC_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(${EXEC_NAME} PROPERTIES
    CXX_STANDARD          11
    CXX_STANDARD_REQUIRED ON
    CXX_CLANG_TIDY        "${CMAKE_CXX_CLANG_TIDY}"
)

if(ADD_SANITIZERS)
    target_compile_options(${EXEC_NAME} PUBLIC -fsanitize=address -g)
    set_target_properties(${EXEC_NAME} PROPERTIES LINK_FLAGS "-fsanitize=address")
endif()

find_package(SFML 2.5.1 REQUIRED
    COMPONENTS
        graphics
        window
        system
        audio
)

include_directories(${SFML_INCLUDE_DIR})
target_link_libraries(${EXEC_NAME} PRIVATE sfml-graphics sfml-window sfml-system sfml-audio)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -Wpedantic -Wcast-qual -Wcast-align -Wconversion \
    -Wsign-promo -Wfloat-equal -Wenum-compare -Wold-style-cast -Wredundant-decls -Wsign-conversion -Wnon-virtual-dtor \
    -Wctor-dtor-privacy -Woverloaded-virtual -Wno-float-equal -O3"
)