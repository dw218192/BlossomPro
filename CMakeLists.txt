cmake_minimum_required (VERSION 3.8)

include($ENV{DEVKIT_LOCATION}/cmake/pluginEntry.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(PROJECT_NAME "BlossomPro")
project (${PROJECT_NAME} CXX)

file(GLOB EXPRESSION_PARSER_SRCS "src/ExpressionParser/*.cpp")
file(GLOB PHYLLOTAXIS_SRCS "src/Phyllotaxis/*.cpp" "src/Phyllotaxis/*.h")

# main plugin srcs
file(GLOB SOURCE_FILES "src/*.cpp" "src/*.h")

add_library(ExpressionParser STATIC ${EXPRESSION_PARSER_SRCS})

target_include_directories(ExpressionParser
	PUBLIC
		${PROJECT_SOURCE_DIR}/include
)

add_library (PhyllotaxisGenerator STATIC ${PHYLLOTAXIS_SRCS})
target_link_libraries (PhyllotaxisGenerator PRIVATE ExpressionParser)

set(LIBRARIES OpenMaya Foundation)

find_package(MtoA)
find_alembic()
build_plugin()
target_link_libraries (${PROJECT_NAME} PhyllotaxisGenerator)