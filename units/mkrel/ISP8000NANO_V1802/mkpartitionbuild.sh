#!/bin/bash
echo "ISP software trim for release"

echo "remove 3a source code..."
rm ../../aaa/a* -rf

echo "remove bufferpool source code..."
rm ../../bufferpool/source -rf
rm ../../bufferpool/include -rf
rm ../../bufferpool/codeanalysis -rf
rm ../../bufferpool/example_app -rf
rm ../../bufferpool/unit_test -rf
rm ../../bufferpool/*.prj -rf

echo "remove cam_engine source code..."
rm ../../cam_engine/source -rf
rm ../../cam_engine/codeanalysis -rf
rm ../../cam_engine/example_app -rf
rm ../../cam_engine/unit_test -rf
rm ../../cam_engine/*.prj -rf

echo "remove cameric_drv source code..."
rm ../../cameric_drv/source -rf
rm ../../cameric_drv/codeanalysis -rf
rm ../../cameric_drv/example_app -rf
rm ../../cameric_drv/unit_test -rf
rm ../../cameric_drv/*.prj -rf

echo "remove hal source code..."
rm ../../hal/source/*.c -rf
rm ../../hal/include* -rf
rm ../../hal/3rd_libs -rf
rm ../../hal/codeanalysis -rf
rm ../../hal/example_app -rf
rm ../../hal/unit_test -rf
rm ../../hal/*.prj -rf

echo "remove som_ctrl source code..."
rm ../../som_ctrl/source -rf
rm ../../som_ctrl/include* -rf
rm ../../som_ctrl/codeanalysis -rf
rm ../../som_ctrl/example_app -rf
rm ../../som_ctrl/unit_test -rf
rm ../../som_ctrl/*.prj -rf

echo "change binary compile configuration"
#sed -i "s/^set (GENERATE_PARTITION_BUILD 1)/#set (GENERATE_PARTITION_BUILD 1)/g" ./*.cmake

echo "binary trim done!"

