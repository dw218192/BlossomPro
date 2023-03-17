cmake_minimum_required (VERSION 3.8)

include($ENV{DEVKIT_LOCATION}/cmake/pluginEntry.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME "BlossomPro")
project (${PROJECT_NAME} CXX)


file(GLOB EXPRESSION_PARSER_SRCS "src/ExpressionParser/*.cpp")
file(GLOB PHYLLOTAXIS_SRCS "src/Phyllotaxis/*.cpp" "src/Phyllotaxis/*.h")
file(GLOB BSPLINE_SURFACE_SRCS "src/BSplineSurface/*.cpp" "src/BSplineSurface/*.h")
file(GLOB RESOURCE_FILES "res/*.qrc")
file(GLOB GLUT_LIBS "$ENV{GLUT_LIB_DIR}/*lib")

# set additional include dir
set(GLM_INCLUDE "include/glm/")
set(BSPLINE_SURFACE_INCLUDE "src/BSplineSurface/")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# main plugin srcs
file(GLOB SOURCE_FILES ${SOURCE_FILES} "src/*.cpp" "src/*.h" "src/Cmds/*.cpp" "src/Widgets/*.cpp")

# test app srcs
file(GLOB EXPRESSION_TEST_SRCS "src/TestApps/*.cpp")

# our libs
add_library(ExpressionParser STATIC ${EXPRESSION_PARSER_SRCS})
target_include_directories(ExpressionParser
	PUBLIC
		${PROJECT_SOURCE_DIR}/include
)
add_library (PhyllotaxisGenerator STATIC ${PHYLLOTAXIS_SRCS})
target_link_libraries (PhyllotaxisGenerator PRIVATE ExpressionParser)

add_library(BSplineSurface STATIC ${BSPLINE_SURFACE_SRCS})
target_include_directories(BSplineSurface
	PUBLIC
		${GLM_INCLUDE}
)

# test apps
add_executable(ExpressionParserTest ${EXPRESSION_TEST_SRCS})
target_include_directories(ExpressionParserTest 
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(ExpressionParserTest PRIVATE ExpressionParser)

# test BSplineSurface
add_executable(TestBSplineSurface "src/TestBSplineSurface/main.cpp")
target_include_directories(TestBSplineSurface 
	PRIVATE
		${BSPLINE_SURFACE_INCLUDE}
		$ENV{GLUT_DIR}
)
target_link_libraries(TestBSplineSurface PRIVATE BSplineSurface)
target_link_libraries(TestBSplineSurface 
	PRIVATE 
		${GLUT_LIBS}
)

# plugin
set(LIBRARIES OpenMaya OpenMayaUI Foundation)

find_package(MtoA)
find_alembic()

build_plugin()

find_package(Qt5 REQUIRED COMPONENTS Widgets Gui OpenGL Core)

# add resource files
target_sources(${PROJECT_NAME}
	PRIVATE 
		${RESOURCE_FILES}
)

# add auto-generated source files
target_include_directories(${PROJECT_NAME} 
	PUBLIC
		${AUTOGEN_BUILD_DIR}
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
)

# complier settings
if (MSVC)
  target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:__cplusplus")
endif ()

# link our libs
target_link_libraries (${PROJECT_NAME} PhyllotaxisGenerator)
# link qt
target_link_libraries (${PROJECT_NAME} Qt5::Widgets Qt5::Gui Qt5::OpenGL Qt5::Core)
# link openGL
find_package(OpenGL REQUIRED)
target_link_libraries (${PROJECT_NAME} opengl32)