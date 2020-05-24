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

find_package(Threads REQUIRED)

set( THIRD_PARTY_LIBS 
    ${OpenCV_LIBS}
)
############### source and test ######################
include_directories( ${PROJECT_SOURCE_DIR}/../include )

#readFIFO
add_executable(readFIFO
readfifo.cpp 
)
target_link_libraries( readFIFO
    ${THIRD_PARTY_LIBS}
    /home/jc-acc/pipeline_file_queue/lib/libfifopipe.a
)

#writeFIFO
add_executable(writeFIFO
writefifo.cpp 
)
target_link_libraries( writeFIFO
    ${THIRD_PARTY_LIBS}
    /home/jc-acc/pipeline_file_queue/lib/libfifopipe.a
)

#matqueue
add_executable(matqueue
queue_test.cpp 
)
target_link_libraries( matqueue
    ${THIRD_PARTY_LIBS}
    /home/jc-acc/pipeline_file_queue/lib/libqueue.a
)
target_link_libraries (matqueue ${CMAKE_THREAD_LIBS_INIT})

#share memory producer
add_executable(shm_producer
producer.cpp 
)
target_link_libraries( shm_producer
    ${THIRD_PARTY_LIBS}
    #/home/jc-acc/pipeline_file_queue/lib/libqueue.a
)
target_link_libraries (shm_producer ${CMAKE_THREAD_LIBS_INIT})

#share memory consumer
add_executable(shm_consumer
consumer.cpp 
)
target_link_libraries( shm_consumer
    ${THIRD_PARTY_LIBS}
    #/home/jc-acc/pipeline_file_queue/lib/libqueue.a
)
target_link_libraries (shm_consumer ${CMAKE_THREAD_LIBS_INIT})

#producer_shmcq
add_executable(producer_shmcq
producer_shmcq.cpp 
)
target_link_libraries( producer_shmcq
    ${THIRD_PARTY_LIBS}
    /home/jc-acc/pipeline_file_queue/lib/libshmcq.a
)

#consumer_shmcq
add_executable(consumer_shmcq
consumer_shmcq.cpp 
)
target_link_libraries( consumer_shmcq
    ${THIRD_PARTY_LIBS}
    /home/jc-acc/pipeline_file_queue/lib/libshmcq.a
)

############### compile lib #### ######################
#add_library( fifopipe
#	${PROJECT_SOURCE_DIR}/src/fifofile.cpp
#)

