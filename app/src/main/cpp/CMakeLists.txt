PROJECT(thirdparty C CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 3.10.2)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

set(jnilibs ${CMAKE_SOURCE_DIR}/../jniLibs)
set(libname IKNX)

include_directories(
        ${CMAKE_SOURCE_DIR}/
        ${CMAKE_SOURCE_DIR}/utils
        ${CMAKE_SOURCE_DIR}/com
        ${CMAKE_SOURCE_DIR}/knx
        ${CMAKE_SOURCE_DIR}/knx/include
        ${CMAKE_SOURCE_DIR}/socket
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
        ${CMAKE_SOURCE_DIR}/socket/*.cpp
        )

add_library( # Sets the name of the library.
        ${libname}
        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${src-files}
        )


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
        )