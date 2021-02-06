message("#### Product Configuration Version: ${ISP_VERSION}")

add_custom_target(product_build_tag ALL
                  COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/${ISP_VERSION}
                  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_CFG_DIR}/${ISP_VERSION}/${PROJECT_CFG_FILE} ${PROJECT_BINARY_DIR}/${ISP_VERSION}/${PROJECT_CFG_FILE}
                  COMMENT "Current product configuration is ${ISP_VERSION}")



#definitions
add_definitions(-DISP8000NANO_BASE)
add_definitions(-DISP_MULTI_NANO)
add_definitions(-DISP_MP_34BIT)
add_definitions(-DISP_SINGLE_DOM_OVER_MP)


add_definitions(-DISP_DVP_PINMAPPING)
add_definitions(-DISP_MIV1)
add_definitions(-DISP_HDR_STITCH)
add_definitions(-DISP_WDR_V3)
add_definitions(-DISP_TPG)
add_definitions(-DISP_MI_BUF_DUMP)
add_definitions(-DISP_COMPAND)
add_definitions(-DISP8000NANO_V1802)
add_definitions(-DISP_VERSION=\"ISP8000NANO_V1802\")
add_definitions(-DISP_FILTER)
add_definitions(-DISP_COMPAND)


#FPGA support on VSI FPGA
#add_definitions(-DFPGA_SUPPORT_PACKAGE)

#OV2775 work mode switch
#add_definitions(-DSENSOR_OV2775_LIN)
#add_definitions(-DSENSOR_OV2775_NAT)

#macros
set (ENABLE_QT 1)
#set (FPGA_SUPPORT_PACKAGE 1)
