cmake_minimum_required( VERSION 2.8 )
project ( FIFOtest )

set( CMAKE_CXX_COMPILER "g++" )
set( CMAKE_BUILD_TYPE "Release" )
set( CMAKE_CXX_FLAGS "-std=c++11 -O3" )

set( EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin )
#set( LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib )

############### dependencies ######################
# OpenCV
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

set( THIRD_PARTY_LIBS 
    ${OpenCV_LIBS}
)
############### source and test ######################
include_directories( ${PROJECT_SOURCE_DIR}/../include )
add_executable(readFIFO
readfifo.cpp 
)
target_link_libraries( readFIFO
    ${THIRD_PARTY_LIBS}
    /home/jc-acc/pipeline_file_queue/lib/libpipe_queue.a
)

add_executable(writeFIFO
writefifo.cpp 
)
target_link_libraries( writeFIFO
    ${THIRD_PARTY_LIBS}
    /home/jc-acc/pipeline_file_queue/lib/libpipe_queue.a
)

############### compile lib #### ######################
#add_library( pipe_queue
#	${PROJECT_SOURCE_DIR}/src/fifofile.cpp
#)

