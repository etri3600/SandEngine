cmake_minimum_required (VERSION 3.8)

##################### Variables ############################
# Change if you want modify path or other values           #
############################################################

# Project
get_filename_component(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
set(PROJECT_NAME SandEngine)


################# CMake Project ############################
# The main options of project                              #
############################################################

project(${PROJECT_NAME} CXX)

# Define Release by default.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: Use Release by default.")
endif(NOT CMAKE_BUILD_TYPE)

# Outputs
set(LIBRARY_OUTPUT_DIR "../Lib/${CMAKE_BUILD_TYPE}")
set(RUNTIME_OUTPUT_DIR "../Bin/${CMAKE_BUILD_TYPE}")

############## Artefacts Output ############################
# Defines outputs , depending BUILD TYPE                   #
############################################################

message(STATUS ${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_DIR}/${LIBRARY_OUTPUT_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_DIR}/${LIBRARY_OUTPUT_DIR}")
set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${PROJECT_DIR}/${RUNTIME_OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_DIR}/${RUNTIME_OUTPUT_DIR}")

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

set_property(GLOBAL PROPERTY USE_FOLDERS TRUE)

function(get_all_targets _result _dir)
    get_property(_subdirs DIRECTORY "${_dir}" PROPERTY SUBDIRECTORIES)
    foreach(_subdir IN LISTS _subdirs)
        get_all_targets(${_result} "${_subdir}")
    endforeach()
    get_property(_sub_targets DIRECTORY "${_dir}" PROPERTY BUILDSYSTEM_TARGETS)
    set(${_result} ${${_result}} ${_sub_targets} PARENT_SCOPE)
endfunction()

function(add_subdirectory_in_folder _subdirectory _folder)
    add_subdirectory(${_subdirectory} ${ARGN})
    get_all_targets(_targets "${_subdirectory}")
    foreach(_target IN LISTS _targets)
        set_target_properties(
            ${_target}
            PROPERTIES FOLDER "${_folder}"
        )
    endforeach()
endfunction()

# Include Library
include_directories(${PROJECT_DIR}/Include)

set(Libraries)

include_directories($ENV{VULKAN_SDK}/Include)
if (MSVC)
  link_directories($ENV{VULKAN_SDK}/Source/lib)
  find_library(vulkan NAMES vulkan-1 HINTS $ENV{VULKAN_SDK}/Source/lib)
else (MSVC)
  link_directories($ENV{VULKAN_SDK}/lib)
  find_library(vulkan NAMES vulkan HINTS $ENV{VULKAN_SDK}/lib)
endif (MSVC)
message(STATUS $ENV{VULKAN_SDK}/lib)
message(STATUS ${vulkan})
list(APPEND Libraries ${vulkan})

add_definitions(-DUNICODE -D_UNICODE)
set(UNICODE true)

add_subdirectory_in_folder(External/assimp-4.1.0 External)
list(APPEND Libraries assimp)
add_dependencies(${PROJECT_NAME} assimp)

add_subdirectory_in_folder(External/DevIL-1.8.0 External)
list(APPEND Libraries IL ILU ILUT)
add_dependencies(${PROJECT_NAME} IL ILU ILUT)

set_target_properties(IL PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_DIR}/${LIBRARY_OUTPUT_DIR}")
set_target_properties(IL PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_DIR}/${LIBRARY_OUTPUT_DIR}")
set_target_properties(IL PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_DIR}/${RUNTIME_OUTPUT_DIR}")

set_target_properties(ILU PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_DIR}/${LIBRARY_OUTPUT_DIR}")
set_target_properties(ILU PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_DIR}/${LIBRARY_OUTPUT_DIR}")
set_target_properties(ILU PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_DIR}/${RUNTIME_OUTPUT_DIR}")

set_target_properties(ILUT PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_DIR}/${LIBRARY_OUTPUT_DIR}")
set_target_properties(ILUT PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_DIR}/${LIBRARY_OUTPUT_DIR}")
set_target_properties(ILUT PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_DIR}/${RUNTIME_OUTPUT_DIR}")

unset(UNICODE)
remove_definitions(-DUNICODE -D_UNICODE)

target_link_libraries(${PROJECT_NAME} PUBLIC ${Libraries})
