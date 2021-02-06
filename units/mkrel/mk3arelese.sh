#!/bin/bash
echo "ISP sw 3a release trim for release"

echo "remove 3a source code..."
rm ../aaa/a* -rf

echo "change 3a binary compile"
sed -i "s/^set (GENERATE_3A_LIB 1)/#set (GENERATE_3A_LIB 1)/g" ../CMakeLists.txt
sed -i "s/^set (GENERATE_3A_LIB 1)/#set (GENERATE_3A_LIB 1)/g" ../cmodel/CMakeLists.txt

echo "3a bin trim done!"
