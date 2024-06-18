cmake_minimum_required(VERSION 3.25)

project(Qcrc VERSION 0.1 LANGUAGES CXX)

# 配置通用编译
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/.cmake/)
include(GetGitRevisionDescription)

# 设置可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
set(APPLICATION_DIR_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# 导入Qt相关依赖包
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# 生成版本信息头文件
set(HEADER_FILE_INFO_VERSION_PATH ${CMAKE_SOURCE_DIR}/helper/InfoVersion.h)
configure_file(
    ${CMAKE_SOURCE_DIR}/.cmake/InfoVersion.h.in
    ${HEADER_FILE_INFO_VERSION_PATH}
)

# inno setup打包脚本
configure_file(
    ${CMAKE_SOURCE_DIR}/.cmake/InstallerScript.iss.in
    ${CMAKE_SOURCE_DIR}/action-cli/InstallerScript.iss
)

# 从当前源目录的src子目录中递归查找.cpp和.h文件
file(GLOB_RECURSE SRC_FILES
    ${CMAKE_SOURCE_DIR}/helper/*.cpp
    ${CMAKE_SOURCE_DIR}/helper/*.h*
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.h*
)

foreach(filepath ${SRC_FILES})
    file(RELATIVE_PATH filename ${CMAKE_CURRENT_LIST_DIR} ${filepath})

    # message(STATUS "Found : ${filename}")
    list(APPEND sources_files ${filepath})
endforeach(filepath)

# 如果是Windows平台,则生成rc文件,还有inno setup脚本文件
set(EXE_VERSION_RC_PATH "")
if(WIN32)
    set(EXE_VERSION_RC_PATH ${CMAKE_CURRENT_BINARY_DIR}/version_${PROJECT_NAME}.rc)
    configure_file(
        ${CMAKE_SOURCE_DIR}/.cmake/version_exe.rc.in
        ${EXE_VERSION_RC_PATH}
    )
endif()

if(WIN32)
    list(APPEND sources_files ${EXE_VERSION_RC_PATH})
endif()

set(PROJECTNAME_QRC_PATH ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.qrc)

if(NOT EXISTS "${PROJECTNAME_QRC_PATH}")
configure_file(
    ${CMAKE_SOURCE_DIR}/.cmake/ProJectName.qrc.in
    ${PROJECTNAME_QRC_PATH}
)
endif()

# 加快qrc编译
qt_add_big_resources(QRC_RESOURCES ${PROJECT_NAME}.qrc)
list(APPEND QRC_RESOURCES ${PROJECT_NAME}.qrc)
set_property(SOURCE ${PROJECT_NAME}.qrc PROPERTY SKIP_AUTORCC ON)
list(APPEND sources_files ${QRC_RESOURCES})

qt_add_executable(${PROJECT_NAME}
    MANUAL_FINALIZATION
    ${sources_files}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/helper
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)


set_target_properties(${PROJECT_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Release也支持日志打印代码位置
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    QT_MESSAGELOGCONTEXT
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DRELEASE_BUILD_NO_LOG)
endif()

# 添加部署脚本
if (CMAKE_BUILD_TYPE MATCHES "Release")
    if(WIN32)
        find_program(QT_DEPLOY_QT NAMES windeployqt)
        add_custom_target(Script-DeployRelease
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/dist
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/dist
            COMMAND ${QT_DEPLOY_QT} --plugindir ${CMAKE_SOURCE_DIR}/dist/plugins --no-translations --compiler-runtime ${CMAKE_SOURCE_DIR}/dist/${PROJECT_NAME}.exe
            COMMENT "Windows Deploying Qt Dependencies After Build........."
            SOURCES ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        )
    endif()
endif()

