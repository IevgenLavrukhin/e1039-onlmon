# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(OnlMon CXX C)

# source code
file(GLOB          sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB     dist_headers ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB non_dist_headers ${PROJECT_SOURCE_DIR}/*LinkDef.h)
list(REMOVE_ITEM dist_headers ${non_dist_headers})

# ROOT dict generation
add_custom_command (
  OUTPUT OnlMon_Dict.cc
  COMMAND rootcint
  ARGS -f OnlMon_Dict.cc -noIncludePaths -inlineInputHeader -c -p
    -I${PROJECT_SOURCE_DIR}/  -I${ROOT_PREFIX}/include/
    ${dist_headers}  ${PROJECT_SOURCE_DIR}/LinkDef.h
  DEPENDS ${dist_headers} ${PROJECT_SOURCE_DIR}/LinkDef.h
)

# ROOT
find_program(ROOTCONF "root-config")
if(ROOTCONF)
  message("-- Detecting ROOT:    found at ${ROOTCONF}")
else()
  message(FATAL_ERROR "-- Detecting ROOT:    not found")
endif()
execute_process(COMMAND root-config --prefix OUTPUT_VARIABLE ROOT_PREFIX  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CFLAGS  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --libs   OUTPUT_VARIABLE ROOT_LINK    OUTPUT_STRIP_TRAILING_WHITESPACE)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -O2 -std=c++0x ${ROOT_CFLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -O2 ${ROOT_CFLAGS}")

add_library(OnlMon SHARED ${sources} OnlMon_Dict.cc)
target_link_libraries(OnlMon fun4all db_svc UtilAna ktracker rs_Reader)
install(TARGETS OnlMon DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
install(
    CODE "file( GLOB pcm_files ${CMAKE_BINARY_DIR}/*_rdict.pcm )"
    CODE "file( INSTALL \${pcm_files} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )"
)

## The `install(...)` commmand above is better than the commands below, because it is executed during installation, not build.
#add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up *_rdict.pcm ${CMAKE_INSTALL_PREFIX}/lib)
#add_dependencies(install_pcm OnlMon)
