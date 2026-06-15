cmake_minimum_required(VERSION 3.20)
project(DirectX12_Game_1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT WIN32)
  message(FATAL_ERROR "This sample currently targets Windows only.")
endif()

add_executable(DirectX12_Game_1 WIN32 src/main.cpp)

target_compile_definitions(DirectX12_Game_1 PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)

target_link_libraries(DirectX12_Game_1 PRIVATE d3d12 dxgi dxguid)
