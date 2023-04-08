cmake_minimum_required (VERSION 3.11)

include($ENV{DEVKIT_LOCATION}/cmake/pluginEntry.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME "BlossomPro")
project (${PROJECT_NAME} CXX)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

# main lib srcs
file(GLOB EXPRESSION_PARSER_SRCS "src/ExpressionParser/*.cpp")
file(GLOB WIDGET_SRCS "src/Widgets/*.cpp")
file(GLOB PHYLLOTAXIS_SRCS "src/Phyllotaxis/*.cpp" "src/Phyllotaxis/*.h")
file(GLOB BSPLINE_SURFACE_SRCS "src/BSplineSurface/*.cpp" "src/BSplineSurface/*.h")
file(GLOB RESOURCE_FILES "res/*.qrc")

# external sources
set(EXT_DIR ${PROJECT_SOURCE_DIR}/ext)
set(GLUT_DIR ${EXT_DIR}/freeglut-3.2.2)
set(LIBSPLINE_DIR ${EXT_DIR}/libspline)

add_library(libspline INTERFACE)
target_include_directories(libspline INTERFACE ${LIBSPLINE_DIR})
target_sources(libspline INTERFACE ${LIBSPLINE_DIR}/spline.h)

set(SRC_ROOT "src")

# set additional include dir
set(GLM_INCLUDE "${EXT_DIR}/glm/include")
set(GLUT_INCLUDE "${GLUT_DIR}/include")
set(BSPLINE_SURFACE_INCLUDE "src/BSplineSurface/")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# main plugin srcs
file(GLOB SOURCE_FILES ${SOURCE_FILES} 
	"src/*.cpp" 
	"src/*.h"
	"src/Cmds/*.cpp" 
	"src/Widgets/*.cpp"
	"src/MayaNodes/*.cpp"
)

# test app srcs
file(GLOB EXPRESSION_TEST_SRCS "src/TestExpressionParser/*.cpp")
file(GLOB WIDGET_TEST_SRCS "src/TestWidgets/*.cpp")
file(GLOB BSPLINE_SURF_TEST_SRCS "src/TestBSplineSurface/*.cpp")

# our libs
add_library(ExpressionParser STATIC ${EXPRESSION_PARSER_SRCS})
target_include_directories(ExpressionParser
	PUBLIC
		${PROJECT_SOURCE_DIR}/include
)

find_package(Qt5 REQUIRED COMPONENTS Widgets Gui OpenGL Core)
find_package(OpenGL REQUIRED)

add_library(CustomWidgets STATIC ${WIDGET_SRCS})
target_include_directories(CustomWidgets
	PUBLIC
		${GLM_INCLUDE}
		${Qt5Widgets_INCLUDE_DIRS}
)
target_link_libraries (CustomWidgets
	PRIVATE
		ExpressionParser
		libspline
)

add_library (PhyllotaxisGenerator STATIC ${PHYLLOTAXIS_SRCS})
target_include_directories (PhyllotaxisGenerator
	PUBLIC
		${GLM_INCLUDE}
)
target_link_libraries (PhyllotaxisGenerator 
	PRIVATE
		ExpressionParser
		CustomWidgets
		libspline
)

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
target_link_libraries(ExpressionParserTest
	PRIVATE
		ExpressionParser
)

add_executable(WidgetTest ${WIDGET_TEST_SRCS})
target_link_libraries(WidgetTest
	PRIVATE
		CustomWidgets
		OpenGL::GL
		PhyllotaxisGenerator
		Qt5::Widgets Qt5::Gui Qt5::OpenGL Qt5::Core
		libspline
)


# test BSplineSurface
# set vars for freeglut
set(FREEGLUT_BUILD_SHARED_LIBS ON)
set(FREEGLUT_BUILD_DEMOS OFF)
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


# add resource files
target_sources(${PROJECT_NAME}
	PRIVATE 
		${RESOURCE_FILES}
)

# add auto-generated source files
target_include_directories(${PROJECT_NAME} 
	PUBLIC
		${AUTOGEN_BUILD_DIR}
		${GLM_INCLUDE}
		${BSPLINE_SURFACE_INCLUDE}	
	PRIVATE
		${SRC_ROOT}
		${PROJECT_SOURCE_DIR}/include
)

# complier settings
if (MSVC)
  target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:__cplusplus")
endif ()

# link our libs
target_link_libraries (${PROJECT_NAME} 
	PhyllotaxisGenerator
	libspline
	BSplineSurface
)

# link qt
target_link_libraries (${PROJECT_NAME} Qt5::Widgets Qt5::Gui Qt5::OpenGL Qt5::Core)
# link openGL
target_link_libraries (${PROJECT_NAME} OpenGL::GL)