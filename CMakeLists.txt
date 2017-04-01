cmake_minimum_required(VERSION 3.5)
set(CMAKE_BUILD_TYPE Debug)

project(relay C CXX)

# == MBED OS 5 settings ==
set(FEATURES netsocket)

add_definitions(
        -DTOOLCHAIN_GCC
        -DCPU_MK82FN256VLL15
        -DTARGET_K82F
        -DTARGET_UBIRCH1
        -DMBED_CONF_RTOS_PRESENT
        -DMBED_CONF_NSAPI_PRESENT
        -DMBED_CONF_EVENTS_PRESENT
        -D__MBED__
        -DDEVICE_RTC
        -DDEVICE_I2C
        -DDEVICE_SPI
        -DDEVICE_SERIAL
        -DDEVICE_TRNG
        -DDEVICE_INTERRUPTIN
        -DMBED_CONF_EVENTS_PRESENT=1
        -DMBED_CONF_RTOS_PRESENT=1
        -DMBED_CONF_PLATFORM_STDIO_BAUD_RATE=115200
        -DMBED_CONF_NSAPI_PRESENT=1
        -DMBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE=115200
        -DMBED_CONF_PLATFORM_STDIO_FLUSH_AT_EXIT=0
        -DMBED_CONF_PLATFORM_STDIO_CONVERT_NEWLINES=0
)

set(MBED_OS
        mbed-os/targets/TARGET_Freescale/TARGET_MCUXpresso_MCUS/TARGET_K82F/TARGET_UBRIDGE
        mbed-os/targets/TARGET_Freescale/TARGET_MCUXpresso_MCUS/TARGET_K82F/device/TOOLCHAIN_GCC_ARM
        mbed-os/targets/TARGET_Freescale/TARGET_MCUXpresso_MCUS/TARGET_K82F/drivers
        mbed-os/targets/TARGET_Freescale/TARGET_MCUXpresso_MCUS/TARGET_K82F/device
        mbed-os/targets/TARGET_Freescale/TARGET_MCUXpresso_MCUS/TARGET_K82F
        mbed-os/targets/TARGET_Freescale/TARGET_MCUXpresso_MCUS/api
        mbed-os/targets/TARGET_Freescale
        mbed-os/rtos/rtx/TARGET_CORTEX_M/TARGET_RTOS_M4_M7
        mbed-os/rtos/rtx/TARGET_CORTEX_M
        mbed-os/rtos
        mbed-os/features
        mbed-os/platform
        mbed-os/hal/storage_abstraction
        mbed-os/hal
        mbed-os/events/enqueue
        mbed-os/events
        mbed-os/drivers
        mbed-os/cmsis
        mbed-os
        )

set(MBED_OS_SRCS "" source/main.cpp)
foreach (DIR in ${MBED_OS})
    if (DIR MATCHES "mbed-os/features")
        foreach(FEATURE in ${FEATURES})
            file(GLOB_RECURSE includes
                    ${DIR}/${FEATURE}/*.cpp ${DIR}/${FEATURE}/*.c ${DIR}/${FEATURE}/*.s ${DIR}/${FEATURE}/*.S)
            list(APPEND MBED_OS_SRCS ${includes})
        endforeach()
    else ()
        file(GLOB includes ${DIR}/*.cpp ${DIR}/*.c ${DIR}/*.s ${DIR}/*.S)
        list(APPEND MBED_OS_SRCS ${includes})
    endif ()
endforeach ()

add_library(mbed-os ${MBED_OS_SRCS})
include_directories(${MBED_OS})
# == END MBED OS 5 ==

add_library(kinetis-lowpower mbed-kinetis-lowpower/kinetis_lowpower.cpp)
target_include_directories(kinetis-lowpower PUBLIC mbed-kinetis-lowpower/)

add_library(mbed-os-quectel-m66-driver
        mbed-os-quectelM66-driver/M66Interface.cpp
        mbed-os-quectelM66-driver/M66ATParser/M66ATParser.cpp
        mbed-os-quectelM66-driver/M66ATParser/BufferedSerial/BufferedPrint.c
        mbed-os-quectelM66-driver/M66ATParser/BufferedSerial/BufferedSerial.cpp
        mbed-os-quectelM66-driver/M66ATParser/BufferedSerial/Buffer/MyBuffer.cpp
        )
target_include_directories(mbed-os-quectel-m66-driver PUBLIC
        mbed-os-quectelM66-driver/M66ATParser/BufferedSerial/Buffer
        mbed-os-quectelM66-driver/M66ATParser/BufferedSerial
        mbed-os-quectelM66-driver/M66ATParser
        mbed-os-quectelM66-driver
        )

add_executable(relay source/main.cpp)
target_link_libraries(relay mbed-os kinetis-lowpower mbed-os-quectel-m66-driver)

add_custom_target(relay-compile ALL
        COMMAND mbed compile --profile mbed-os/tools/profiles/debug.json
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})