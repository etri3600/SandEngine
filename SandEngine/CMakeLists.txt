cmake_minimum_required (VERSION 3.8)

##################### Variables ############################
# Change if you want modify path or other values           #
############################################################

# Project
get_filename_component(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
set(PROJECT_NAME SandEngine)

# Outputs
set(OUTPUT_DEBUG ../Lib)
set(OUTPUT_RELEASE ../Lib)

################# CMake Project ############################
# The main options of project                              #
############################################################

project(${PROJECT_NAME} CXX)

# Define Release by default.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: Use Release by default.")
endif(NOT CMAKE_BUILD_TYPE)

############## Artefacts Output ############################
# Defines outputs , depending BUILD TYPE                   #
############################################################

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_DIR}/${OUTPUT_DEBUG}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_DIR}/${OUTPUT_DEBUG}")
  set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${PROJECT_DIR}/${OUTPUT_DEBUG}")
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_DIR}/${OUTPUT_RELEASE}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_DIR}/${OUTPUT_RELEASE}")
  set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${PROJECT_DIR}/${OUTPUT_RELEASE}")
endif()

# Messages
message("${PROJECT_NAME}: MAIN PROJECT: ${CMAKE_PROJECT_NAME}")
message("${PROJECT_NAME}: CURR PROJECT: ${CMAKE_CURRENT_SOURCE_DIR}")
message("${PROJECT_NAME}: CURR BIN DIR: ${CMAKE_CURRENT_BINARY_DIR}")

############### Files & Targets ############################
# Files of project and target to build                     #
############################################################

# Source Files
set(SRC_FILES
    ./Animation.cpp
    ./DirectX12.cpp
    ./DirectX12Device.cpp
    ./DX12DescriptorHeapAllocator.cpp
    ./DX12Helper.cpp
    ./DX12InputVertexAttributes.cpp
    ./DX12ResourceAllocator.cpp
    ./DX12Pipeline.cpp
    ./DX12Resources.cpp
    ./DX12RootSignature.cpp
    ./Graphics.cpp
    ./Image.cpp
    ./ImageLoader.cpp
    ./Math.cpp
    ./ModelLoader.cpp
    ./ModelStruct.cpp
    ./Node.cpp
    ./Scene.cpp
    ./SceneManager.cpp
    ./Utils.cpp
    ./Vulkan.cpp
    ./VulkanDevice.cpp
    ./Windows.cpp
)
source_group("Sources" FILES ${SRC_FILES})

# Header Files
set(HEADERS_FILES
    SceneObj.h
    Lighting.h
    SandEngine.h
    Animation.h
    Bone.h
    DirectX12.h
    DirectX12Device.h
    DX12DescriptorHeapAllocator.h
    DX12Helper.h
    DX12InputVertexAttributes.h
    DX12ResourceAllocator.h
    DX12Pipeline.h
    DX12Resources.h
    DX12RootParameter.h
    DX12RootSignature.h
    Graphics.h
    GraphicsInterface.h
    SHeader.h
    Image.h
    Input.h
    InputManager.h
    Linux.h
    Loader.h
    Material.h
    Math.h
    ModelLoader.h
    ModelStruct.h
    ConstantBuffer.h
    PlatformManager.h
    PlatformSystem.h
    Scene.h
    SceneManager.h
    Node.h
    Singletone.h
    ImageLoader.h
    Time.h
    Utils.h
    Vulkan.h
    VulkanDevice.h
    Windows.h
)
source_group("Headers" FILES ${HEADERS_FILES})

# Include Library
include_directories($ENV{VULKAN_SDK}/Include)

include_directories(${PROJECT_DIR}/Include)
link_directories(${PROJECT_DIR}/Lib)
link_directories($ENV{VULKAN_SDK}/Source/lib)
link_directories(${PROJECT_DIR}/Lib/IL)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	link_directories(${PROJECT_DIR}/Lib/assimp/Debug)
else()
	link_directories(${PROJECT_DIR}/Lib/assimp/Release)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	find_library(assimp NAMES assimp-vc140-mt HINTS ${PROJECT_DIR}/Lib/assimp/Debug)
else()
	find_library(assimp NAMES assimp-vc140-mt HINTS ${PROJECT_DIR}/Lib/assimp/Release)
endif()
find_library(DevIL NAMES DevIL HINTS ${PROJECT_DIR}/Lib/IL)
find_library(ILU NAMES ILU HINTS ${PROJECT_DIR}/Lib/IL)
find_library(ILUT NAMES ILUT HINTS ${PROJECT_DIR}/Lib/IL)
find_library(vulkan NAMES vulkan-1 HINTS $ENV{VULKAN_SDK}/Source/lib)

# Add library to build.
add_library(${PROJECT_NAME} STATIC
   ${SRC_FILES} ${HEADERS_FILES}
)

######################### Flags ############################
# Defines Flags for Windows and Linux                      #
############################################################

if(NOT MSVC)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif(NOT MSVC)

# Preprocessor definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE UNICODE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE )
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /JMC /GS /W3 /Zc:wchar_t /Gm- /Od /Ob0 /Gd /sdl /Zc:inline /fp:precise /WX- /Zc:forScope /RTC1 /MDd /FC /Fa /Zi /EHsc) 
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE )
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /GS /GL /W3 /Gy /Zc:wchar_t /Gm- /O2 /Gd /Oi /MD /FC /Fa /EHsc)
    endif()
endif()

########### Link & Dependencies ############################
# Add project dependencies and Link to project             #
############################################################

target_link_libraries(${PROJECT_NAME} PUBLIC ${assimp} ${DevIL} ${ILU} ${ILUT} ${vulkan})
