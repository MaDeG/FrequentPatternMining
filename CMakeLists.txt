cmake_minimum_required(VERSION 3.12)
project(FrequentPatternMining)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost 1.40 COMPONENTS program_options REQUIRED)
find_package(OpenMP REQUIRED)

#file(GLOB FrequentPatternMining_SRC "src/*.h" "src/*.cpp")
#message("Found source files: ${FrequentPatternMining_SRC}")
#add_executable(FrequentPatternMining ${FrequentPatternMining_SRC})
#file(GLOB FrequentPatternMining_SRC CONFIGURE_DEPENDS "src/*.h" "src/*.cpp")

add_executable(FrequentPatternMining src/FileOrderedReader.cpp src/FileOrderedReader.h
                                     src/FPTreeNode.cpp src/FPTreeNode.h
                                     src/FPTreeManager.cpp src/FPTreeManager.h
                                     src/HeaderTable.cpp src/HeaderTable.h
                                     src/HeaderEntry.cpp src/HeaderEntry.h
                                     src/FrequentItemsets.cpp src/FrequentItemsets.h
                                     src/Params.h
                                     src/main.cpp src/Utils.cpp)
target_link_libraries(FrequentPatternMining Boost::program_options OpenMP::OpenMP_CXX)