#!/bin/bash
echo "Dewarp software trim for release"

echo "remove dewarp source code..."
rm ../hal/*.cpp -rf
rm ../parser/*.cpp -rf

echo "change binary compile configuration"
sed -i "s/^set (GENERATE_PARTITION_BUILD 1)/#set (GENERATE_PARTITION_BUILD 1)/g" ../CMakeLists.txt
sed -i "s/^set (GENERATE_PARTITION_BUILD 1)/#set (GENERATE_PARTITION_BUILD 1)/g" ../../mediacontrol/CMakeLists.txt
sed -i "s?dewarp/hal dewarp.hal.out?dewarp/proprietories/hal dewarp.hal.out?" ../../mediacontrol/CMakeLists.txt
sed -i "s?dewarp/parser dewarp.parser.out?dewarp/proprietories/parser dewarp.parser.out?" ../../mediacontrol/CMakeLists.txt


echo "binary trim done!"

