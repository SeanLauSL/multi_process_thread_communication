cmake_minimum_required( VERSION 2.8 )
project ( pipeline_queue_modules )

set( CMAKE_CXX_COMPILER "g++" )
set( CMAKE_BUILD_TYPE "Release" )
set( CMAKE_CXX_FLAGS "-std=c++11 -O3" )

#set( EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib )

############### dependencies ######################
# OpenCV
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

set( THIRD_PARTY_LIBS 
    ${OpenCV_LIBS}
)
############### source and test ######################
include_directories( ${PROJECT_SOURCE_DIR}/include )
#add_executable(exe_name
#src.cpp 
#)

#target_link_libraries( exe_name
#    ${THIRD_PARTY_LIBS}
#)
############### compile lib #### ######################
add_library( fifopipe
	${PROJECT_SOURCE_DIR}/src/fifofile.cpp
)

add_library( queue
	${PROJECT_SOURCE_DIR}/src/capThread.cpp
)

add_library( shmcq
	${PROJECT_SOURCE_DIR}/src/shmcirqueue.cpp
)
