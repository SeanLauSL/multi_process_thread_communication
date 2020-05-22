#!/bin/bash
echo "archiving queue funtions..."

g++ -c ../src/capThread.cpp -I ../include `pkg-config opencv --libs --cflags opencv` -pthread
ar cr libqueue.a *.o #cr标志告诉ar将object文件封装
rm *.o #clean .o files

echo "libqueue.a completed."

