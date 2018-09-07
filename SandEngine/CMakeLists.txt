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

include_directories(${PROJECT_DIR})

set(ASSETS_FILES
	Assets/Animation.cpp
	Assets/Image.cpp
	Assets/ModelStruct.cpp
	Assets/Lighting.h
    Assets/Animation.h
    Assets/Bone.h
	Assets/Image.h
	Assets/Material.h
    Assets/ModelStruct.h
    Assets/ConstantBuffer.h
	Assets/Lighting.h
)

set(GRAPHICS_FILES
    Graphics/DirectX12/DirectX12.cpp
    Graphics/DirectX12/DirectX12Device.cpp
    Graphics/DirectX12/DX12DescriptorHeapAllocator.cpp
	Graphics/DirectX12/DX12Helper.cpp
    Graphics/DirectX12/DX12InputVertexAttributes.cpp
    Graphics/DirectX12/DX12ResourceAllocator.cpp
    Graphics/DirectX12/DX12Pipeline.cpp
    Graphics/DirectX12/DX12Resources.cpp
    Graphics/DirectX12/DX12RootSignature.cpp
    Graphics/Graphics.cpp
    Graphics/Vulkan/Vulkan.cpp
    Graphics/Vulkan/VulkanDevice.cpp
	Graphics/DirectX12/DirectX12.h
    Graphics/DirectX12/DirectX12Device.h
    Graphics/DirectX12/DX12DescriptorHeapAllocator.h
    Graphics/DirectX12/DX12Helper.h
    Graphics/DirectX12/DX12InputVertexAttributes.h
    Graphics/DirectX12/DX12ResourceAllocator.h
    Graphics/DirectX12/DX12Pipeline.h
    Graphics/DirectX12/DX12Resources.h
    Graphics/DirectX12/DX12RootParameter.h
    Graphics/DirectX12/DX12RootSignature.h
    Graphics/Graphics.h
    Graphics/GraphicsInterface.h
    Graphics/Vulkan/Vulkan.h
    Graphics/Vulkan/VulkanDevice.h
)

set(INPUT_FILES
    Input/Input.h
    Input/InputManager.h
)

set(LOADER_FILES
	Loader/ImageLoader.cpp
	Loader/ModelLoader.cpp
	Loader/Loader.h
	Loader/ModelLoader.h
	Loader/ImageLoader.h
)

set(PLATFORM_FILES
	Platform/Windows.cpp
	Platform/Linux.h
    Platform/PlatformManager.h
    Platform/PlatformSystem.h
	Platform/Windows.h
)

set(SCENE_FILES
    Scene/Node.cpp
    Scene/Scene.cpp
    Scene/SceneManager.cpp
	Scene/SceneObj.h
    Scene/Scene.h
    Scene/SceneManager.h
    Scene/Node.h
)

set(FILES
    SMath.cpp
    SUtils.cpp
    SandEngine.h
    SHeader.h
    SMath.h
    Singletone.h
    STime.h
    SUtils.h
)
source_group("Files" FILES ${FILES})

# Add library to build.
add_library(${PROJECT_NAME} STATIC
   ${FILES} ${ASSETS_FILES} ${GRAPHICS_FILES} ${INPUT_FILES} ${LOADER_FILES} ${PLATFORM_FILES} ${SCENE_FILES}
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

# Include Library
include_directories(${PROJECT_DIR}/Include)

include_directories($ENV{VULKAN_SDK}/Include)
link_directories($ENV{VULKAN_SDK}/Source/lib)
find_library(vulkan NAMES vulkan-1 HINTS $ENV{VULKAN_SDK}/Source/lib)

add_definitions(-DUNICODE -D_UNICODE)

add_subdirectory(External/assimp-4.1.0)
add_subdirectory(External/DevIL-1.8.0)

remove_definitions(-DUNICODE -D_UNICODE)

target_link_libraries(${PROJECT_NAME} PUBLIC ${Assimp} ${ImageLib} ${vulkan})
