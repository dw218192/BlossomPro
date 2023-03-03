cmake_minimum_required (VERSION 3.8)

include($ENV{DEVKIT_LOCATION}/cmake/pluginEntry.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME "BlossomPro")
project (${PROJECT_NAME} CXX)

file(GLOB EXPRESSION_PARSER_SRCS "src/ExpressionParser/*.cpp")
file(GLOB PHYLLOTAXIS_SRCS "src/Phyllotaxis/*.cpp" "src/Phyllotaxis/*.h")


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# main plugin srcs
file(GLOB SOURCE_FILES ${SOURCE_FILES} "src/*.cpp" "src/*.h")

# our libs
add_library(ExpressionParser STATIC ${EXPRESSION_PARSER_SRCS})
target_include_directories(ExpressionParser
	PUBLIC
		${PROJECT_SOURCE_DIR}/include
)
add_library (PhyllotaxisGenerator STATIC ${PHYLLOTAXIS_SRCS})
target_link_libraries (PhyllotaxisGenerator PRIVATE ExpressionParser)

# plugin
set(LIBRARIES OpenMaya OpenMayaUI Foundation)

find_package(MtoA)
find_alembic()

build_plugin()

find_package(Qt5 REQUIRED COMPONENTS Widgets Gui Core)

target_include_directories(${PROJECT_NAME} 
	PUBLIC
		${AUTOGEN_BUILD_DIR}
)

# complier settings
if (MSVC)
  target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:__cplusplus")
endif ()

# link our libs
target_link_libraries (${PROJECT_NAME} PhyllotaxisGenerator)
# link qt
target_link_libraries (${PROJECT_NAME} Qt5::Widgets Qt5::Gui Qt5::Core)