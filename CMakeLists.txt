cmake_minimum_required (VERSION 3.8)

include($ENV{DEVKIT_LOCATION}/cmake/pluginEntry.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME "BlossomPro")
project (${PROJECT_NAME} CXX)

# main lib srcs
file(GLOB EXPRESSION_PARSER_SRCS "src/ExpressionParser/*.cpp")
file(GLOB PHYLLOTAXIS_SRCS "src/Phyllotaxis/*.cpp" "src/Phyllotaxis/*.h")
file(GLOB BSPLINE_SURFACE_SRCS "src/BSplineSurface/*.cpp" "src/BSplineSurface/*.h")
file(GLOB RESOURCE_FILES "res/*.qrc")

# external sources
set(EXT_DIR "ext")
set(GLUT_DIR ${EXT_DIR}/freeglut-3.2.2)

# set additional include dir
set(GLM_INCLUDE "${EXT_DIR}/glm/include")
set(GLUT_INCLUDE "${GLUT_DIR}/include")
set(BSPLINE_SURFACE_INCLUDE "src/BSplineSurface/")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# main plugin srcs
file(GLOB SOURCE_FILES ${SOURCE_FILES} "src/*.cpp" "src/*.h" "src/Cmds/*.cpp" "src/Widgets/*.cpp")

# test app srcs
file(GLOB EXPRESSION_TEST_SRCS "src/TestApps/*.cpp")
file(GLOB BSPLINE_SURF_TEST_SRCS "src/TestBSplineSurface/main.cpp")

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
# set vars for freeglut
set(FREEGLUT_BUILD_SHARED_LIBS ON)
add_subdirectory(${GLUT_DIR})

add_executable(TestBSplineSurface ${BSPLINE_SURF_TEST_SRCS})
add_dependencies(TestBSplineSurface freeglut)
target_include_directories(TestBSplineSurface 
	PRIVATE
		${BSPLINE_SURFACE_INCLUDE}
		${GLUT_INCLUDE}
)
target_link_libraries(TestBSplineSurface 
	PRIVATE 
		freeglut
		BSplineSurface
)
# copy ${PROJECT_BINARY_DIR}/${GLUT_DIR}/bin/freeglut*.dll to ${PROJECT_BINARY_DIR}
file(GLOB FREEGLUT_DLLS "${PROJECT_BINARY_DIR}/${GLUT_DIR}/bin/freeglut*.dll")
if(FREEGLUT_DLLS)
	foreach(FREEGLUT_DLL IN LISTS FREEGLUT_DLLS)
        add_custom_command(TARGET TestBSplineSurface POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                    ${FREEGLUT_DLL}
                    $<TARGET_FILE_DIR:TestBSplineSurface>)
    endforeach()
endif()

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