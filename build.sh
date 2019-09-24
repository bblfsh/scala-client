#!/bin/bash

PROTO_DIR="src/main/proto"
SDK_MAJOR="v3"
SDK_VERSION="${SDK_MAJOR}.1.0"
LIBUAST_VERSION="3.4.2"
# sdk-x.y.z
UNZIP_DIR="sdk-${SDK_VERSION:1}"
BBLFSH_PROTO="${PROTO_DIR}/github.com/bblfsh"
SDK_PROTO="${BBLFSH_PROTO}/sdk/${SDK_MAJOR}"
CPP_FLAGS="-shared -Wall -fPIC -O2 -std=c++11"

function setOSEnv {
    case $OSTYPE in
        "linux"*)
            OS="linux"
            COMPILER=g++
            FLAGS="-Wl,-Bsymbolic ${CPP_FLAGS}"
            OS_HEADERS="${JAVA_HOME}/include/linux"
            LIBSCALAUAST_FMT=".so"
            LIBUAST_FMT=".a"
            ;;
        "darwin"*)
            OS="darwin"
            COMPILER=g++
            FLAGS=${CPP_FLAGS}
            OS_HEADERS="${JAVA_HOME}/include/darwin"
            LIBSCALAUAST_FMT=".dylib"
            LIBUAST_FMT=".a"
            ;;
        "msys"*)
            OS="windows"
            COMPILER=g++
            FLAGS="-stdlib=libc++ ${CPP_FLAGS}"
            OS_HEADERS="${JAVA_HOME}/include/win32"
            LIBSCALAUAST_FMT=".dll"
            LIBUAST_FMT=".lib"
            ;;
        *)
            echo "Not recognized operating system $OSTYPE"
            exit -1
            ;;
    esac
}

function cleanFiles {
    echo "[clean] Cleaning downloads and compilation files..."
    rm -rf src/main/resources/{lib,libuast}
    rm -rf ${UNZIP_DIR}
    rm -rf ${BBLFSH_PROTO}
    rm -rf libuast
    rm -f libuast-bin.tar.gz
}

# Downloads and move SDK files to appropriate directory,
# including protocol buffer files
function getSDKFiles {
    echo "[sdk] Downloading and installing SDK${SDK_MAJOR} protocol buffer files..."
    TAR_FILE=${SDK_VERSION}.tar.gz
    mkdir -p ${SDK_PROTO}/protocol && \
    mkdir -p ${SDK_PROTO}/uast/role && \
    curl -SL https://github.com/bblfsh/sdk/archive/${SDK_VERSION}.tar.gz -o ${TAR_FILE} && \
    tar xzf ${TAR_FILE} && \
    cp ${UNZIP_DIR}/protocol/driver.proto ${SDK_PROTO}/protocol/ && \
    cp ${UNZIP_DIR}/uast/role/generated.proto ${SDK_PROTO}/uast/role  && \
    rm -rf ${UNZIP_DIR} ${TAR_FILE}

    echo "[sdk] Done unpacking SDK"
}

# Checks that the library has been downloaded correctly
function checkLibuastDownload {
    find src/main/resources
    nm src/main/resources/libuast/libuast${LIBUAST_FMT} | grep -c UastDecode > /dev/null || exit -2
    nm src/main/resources/libuast/libuast${LIBUAST_FMT} | wc -l > /dev/null || exit -2
}

# Downloads and moves libuast to appropriate directory
function getLibuast {
    echo "[libuast] Downloading libuast binary"

     GH_URL="https://github.com/bblfsh/libuast"
     BINARY_RELEASE_URL="${GH_URL}/releases/download/v${LIBUAST_VERSION}/libuast-${OS}-amd64.tar.gz"
     TAR_FILE=libuast-bin.tar.gz

     curl -sL ${BINARY_RELEASE_URL} -o ${TAR_FILE} && \
     tar xzf libuast-bin.tar.gz && \
     mv ${OS}-amd64 libuast && \
     mkdir -p src/main/resources && \
     mv libuast src/main/resources && \
     rm -f ${TAR_FILE} && \
     checkLibuastDownload
     echo "[libuast] Done downloading libuast"
}

# Compiles the JNI part
function compileNativeCode {
    echo "[native-code] Compiling libuast bindings..."

    OUT_FOLDER=src/main/resources/lib/
    SRC_FOLDER="src/main/native"
    SRC_FILES="${SRC_FOLDER}/org_bblfsh_client_v2_libuast_Libuast.cc ${SRC_FOLDER}/jni_utils.cc"

    mkdir -p ${OUT_FOLDER} && \
    ${COMPILER} ${FLAGS} \
        -I/usr/include -I"${JAVA_HOME}/include/" -I"${OS_HEADERS}"  \
        -Isrc/main/resources/libuast \
        -o ${OUT_FOLDER}/libscalauast${LIBSCALAUAST_FMT} \
        ${SRC_FILES} \
        src/main/resources/libuast/libuast${LIBUAST_FMT} && \
    find ${OUT_FOLDER}

    echo "[native-code] Done compiling libuast bindings..."
}

function compileScalaCode {
    echo "[scala-code] Compiling library uber .jar"
    ./sbt assembly
    echo "[scala-code] Done compiling!"
}

# Correctly set compilation environment for host OS
setOSEnv

# Parse arguments, execution depends on the order
# we feed the arguments to the script
function usage() {
    echo "Usage: $0 [--clean|--get-dependencies|--native|--all]"
    exit -3
}

for arg in "$@"; do
    case $arg in
        "--clean")
            cleanFiles
            ;;
        "--get-dependencies")
            getSDKFiles
            getLibuast
            ;;
        "--native")
            compileNativeCode
            ;;
        "--all")
            compileNativeCode
            compileScalaCode
            ;;
        *)
            echo "Wrong argument: $arg"
            usage
            ;;
    esac
done
