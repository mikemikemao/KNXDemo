PROJECT(thirdparty C CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 3.10.2)

include(CheckStructHasMember)
include(CheckSymbolExists)
list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
check_symbol_exists(sendmmsg sys/socket.h HAVE_SENDMMSG_API)
check_symbol_exists(recvmmsg sys/socket.h HAVE_RECVMMSG_API)
check_struct_has_member("struct mmsghdr" msg_hdr sys/socket.h HAVE_MMSG_HDR)
if(HAVE_MMSG_HDR)
    add_definitions(-DHAVE_MMSG_HDR)
endif()
if(HAVE_SENDMMSG_API)
    add_definitions(-DHAVE_SENDMMSG_API)
endif()
if(HAVE_RECVMMSG_API)
    add_definitions(-DHAVE_RECVMMSG_API)
endif()

if(DEFINED SOCKET_DEFAULT_BUF_SIZE)
    if (SOCKET_DEFAULT_BUF_SIZE EQUAL 0)
        message(STATUS "Socket default buffer size is not set, use the kernel default value")
    else()
        message(STATUS "Socket default buffer size is set to ${SOCKET_DEFAULT_BUF_SIZE}")
    endif ()
    add_definitions(-DSOCKET_DEFAULT_BUF_SIZE=${SOCKET_DEFAULT_BUF_SIZE})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
set(jnilibs ${CMAKE_SOURCE_DIR}/../jniLibs)
set(libname IKNX)

include_directories(
        ${CMAKE_SOURCE_DIR}/
        ${CMAKE_SOURCE_DIR}/utils
        ${CMAKE_SOURCE_DIR}/com
        ${CMAKE_SOURCE_DIR}/knx
        ${CMAKE_SOURCE_DIR}/knx/include
        ${CMAKE_SOURCE_DIR}/test
        ${CMAKE_SOURCE_DIR}/knxcore/address
        ${CMAKE_SOURCE_DIR}/knxcore/communication
        ${CMAKE_SOURCE_DIR}/knxcore/config
        ${CMAKE_SOURCE_DIR}/zltoolkit
        ${CMAKE_SOURCE_DIR}/zltoolkit/Network
        ${CMAKE_SOURCE_DIR}/zltoolkit/Poller
        ${CMAKE_SOURCE_DIR}/zltoolkit/Thread
        ${CMAKE_SOURCE_DIR}/zltoolkit/Util
        ${CMAKE_SOURCE_DIR}/zltoolkit/win32
)

link_directories(
        ${jnilibs}/${ANDROID_ABI}

)

file(GLOB src-files
        ${CMAKE_SOURCE_DIR}/*.cpp
        ${CMAKE_SOURCE_DIR}/utils/*.cpp
        ${CMAKE_SOURCE_DIR}/com/*.cpp
        ${CMAKE_SOURCE_DIR}/knx/*.cpp
        ${CMAKE_SOURCE_DIR}/knx/proto/*.c
        ${CMAKE_SOURCE_DIR}/test/*.cpp
        ${CMAKE_SOURCE_DIR}/knxcore/*.cpp
        ${CMAKE_SOURCE_DIR}/knxcore/communication/*.cpp
        ${CMAKE_SOURCE_DIR}/knxcore/address/*.cpp
        ${CMAKE_SOURCE_DIR}/knxcore/config/*.cpp
        ${CMAKE_SOURCE_DIR}/zltoolkit
        ${CMAKE_SOURCE_DIR}/zltoolkit/Network/*.cpp
        ${CMAKE_SOURCE_DIR}/zltoolkit/Poller/*.cpp
        ${CMAKE_SOURCE_DIR}/zltoolkit/Thread/*.cpp
        ${CMAKE_SOURCE_DIR}/zltoolkit/Util/*.cpp
        ${CMAKE_SOURCE_DIR}/zltoolkit/win32/*.cpp
        )

add_library( # Sets the name of the library.
        ${libname}
        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${src-files}
        )


#add_library(
#        libsrt
#        SHARED
#        IMPORTED
#)

#指明 so 库的路径，CMAKE_SOURCE_DIR 表示 CMakeLists.txt 的路径
#set_target_properties(
#        libsrt
#        PROPERTIES IMPORTED_LOCATION
#        ${CMAKE_SOURCE_DIR}/lib/ezlink/libsrt.so)


set(native-libs
        android
        log
        mediandk
        binder_ndk
        m
        z
        )

target_link_libraries( # Specifies the target library.
        ${libname}
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        ${native-libs}
        #libsrt
        )
