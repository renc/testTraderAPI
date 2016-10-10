cmake_minimum_required(VERSION 2.8.11)

project(testTraderApi)

set (APPNAME testTraderApi) 

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Tell CMake to create the executable
add_executable(${APPNAME} testTraderApi.cpp )

target_link_libraries(${APPNAME} 
	"${PROJECT_SOURCE_DIR}\\v6.3.6_20150515_traderapi_win64\\tradeapidll\\thostmduserapi.lib"
	"${PROJECT_SOURCE_DIR}\\v6.3.6_20150515_traderapi_win64\\tradeapidll\\thosttraderapi.lib")

