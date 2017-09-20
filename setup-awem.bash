#! /bin/bash

echo "setup-awem.bash starts"

rm src/CMakeLists.txt
ln -s /opt/ros/kinetic/share/catkin/cmake/toplevel.cmake src/CMakeLists.txt

rm -rf build
rm -rf devel

catkin_make

echo "setup-awem.bash completes"
