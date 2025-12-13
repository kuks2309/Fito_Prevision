cmake_minimum_required(VERSION 3.16)

# vcpkg toolchain (vcpkg 사용 시 필수)
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
elseif(EXISTS "D:/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

project(VisionComparison VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt5 설정
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt5 찾기
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui Network)

# OpenCV 찾기
# 기존 D:\OpenCV 사용 (vc15 버전)
set(OpenCV_DIR "D:/OpenCV/opencv/build/x64/vc15/lib")
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")

# Halcon 설정 (경로는 실제 설치 경로로 수정 필요)
set(HALCON_DIR "C:/Program Files/MVTec/HALCON-20.11")
if(EXISTS ${HALCON_DIR})
    include_directories(${HALCON_DIR}/include)
    link_directories(${HALCON_DIR}/lib/x64-win64)
else()
    message(WARNING "Halcon directory not found: ${HALCON_DIR}")
endif()

# MultiCam 설정 (경로는 실제 설치 경로로 수정 필요)
set(MULTICAM_DIR "C:/Program Files/Euresys/MultiCam")
if(EXISTS ${MULTICAM_DIR})
    include_directories(${MULTICAM_DIR}/include)
    link_directories(${MULTICAM_DIR}/lib)
else()
    message(WARNING "MultiCam directory not found: ${MULTICAM_DIR}")
endif()

# 소스 파일
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/ImageViewer.cpp
    src/ImageProcessingPanel.cpp
    src/EdgeProcessingWidget.cpp
    src/LineDetectionWidget.cpp
    src/AIPrevisionWidget.cpp
    src/TEEDClient.cpp
    # src/ControlPanel.cpp
    # src/VisionWrapper/VisionPreWrapper.cpp
    # src/VisionWrapper/VisionPreFITOWrapper.cpp

    # Vision 소스 (독립 프로젝트용 간소화 버전)
    src/Vision/Fnc_Vision_Pre_FITO_Standalone.cpp
)

set(HEADERS
    src/MainWindow.h
    src/ImageViewer.h
    src/ImageProcessingPanel.h
    src/EdgeProcessingWidget.h
    src/LineDetectionWidget.h
    src/AIPrevisionWidget.h
    src/TEEDClient.h
    # src/ControlPanel.h
    # src/VisionWrapper/VisionPreWrapper.h
    # src/VisionWrapper/VisionPreFITOWrapper.h
    src/Vision/Fnc_Vision_Pre_FITO.h
)

set(UI_FILES
    ui/MainWindow.ui
    ui/EdgeProcessingWidget.ui
    ui/LineDetectionWidget.ui
    ui/AIPrevisionWidget.ui
    # ui/ControlPanel.ui
    # ui/AnalysisPanel.ui
)

# UI 파일 경로 설정
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

# 실행 파일 생성
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
)

# 링크 라이브러리
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Network
)

# OpenCV 라이브러리
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
message(STATUS "OpenCV found: ${OpenCV_VERSION}")

# Halcon 라이브러리 (있는 경우에만)
if(EXISTS ${HALCON_DIR})
    target_link_libraries(${PROJECT_NAME}
        halcon
        halconcpp
    )
endif()

# MultiCam 라이브러리 (있는 경우에만)
if(EXISTS ${MULTICAM_DIR})
    target_link_libraries(${PROJECT_NAME}
        MultiCam
        VirtualFG40CL
    )
endif()

# Include 디렉토리
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Vision
    ${OpenCV_INCLUDE_DIRS}
    # D:/Execute/SCTC_CUTTER/SCTCApplication/Src/ControlLogic/Cutter/PM_Function/Functional_Layer
)

# Windows 런타임 라이브러리 설정
if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    # UTF-8 소스 파일 처리
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()

# 설치 규칙 (선택사항)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
