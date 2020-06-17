#!/bin/bash

pwd=$(PWD)

build_script_path="${pwd}/../tools/macos/build.sh"
if [[ ! -f ${build_script_path} ]]; then
    echo "this script should be run from the build directory which should be able to reach the script via ${build_script_path}"
    exit 1
fi

if [[ ! -d dependencies ]]; then
    mkdir dependencies
fi

if [[ ! -d dependencies/include/boost ]]; then
    boost_version=72
    boost="boost_1_${boost_version}_0"
    if [[ ! -d ${boost} ]]; then
        if [[ ! -f  ${boost}.tar.gz ]]; then
            curl -O -L https://dl.bintray.com/boostorg/release/1.${boost_version}.0/source/${boost}.tar.gz || exit 1
        fi
        tar -xvzf ${boost}.tar.gz || exit 1
    fi
    cd ${boost}
    if [[ -f build_boost.log ]]; then
        rm build_boost.log
    fi
    ./bootstrap.sh --prefix=${pwd}/dependencies threading=multi --with-toolset=clang &&
    ./b2 -j12 --prefix=${pwd}/dependencies threading=multi link=static runtime-link=static --with-system --with-thread --with-chrono --with-filesystem --with-log --with-locale --with-regex --with-date_time --with-coroutine --with-context cxxflags="-stdlib=libc++ -std=c++11" linkflags="-stdlib=libc++ -std=c++11" install > build_boost.log 2>&1 
    rc=$?
    cd ..
    if [[ ${rc} -ne 0 ]]; then
        exit 1
    fi
fi

brew install glew tbb sfml icu4c || exit 1

if [[ ! -f dependencies/cef/include/cef_version.h ]]; then
    mkdir -p dependencies/cef
    cef="cef_binary_74.1.19%2Bgb62bacf%2Bchromium-74.0.3729.157_macosx64.tar.bz2"
    if [[ ! -f ${cef} ]]; then
        curl -O -L http://opensource.spotify.com/cefbuilds/${cef} || exit 1
    fi
    tar -xvzf ${cef} -C dependencies/cef --strip-components=1 || exit 1
fi

if [[ ! -f dependencies/cef/Release/libcef_dll_wrapper.a ]]; then
    cd dependencies/cef
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX:string=$(pwd)/dependencies/cef/Release -DCMAKE_BUILD_TYPE=Release ../ && 
    make -j12 &&
    cp libcef_dll_wrapper/libcef_dll_wrapper.a ../Release
    rc=$?
    cd ../../..
    if [[ ${rc} -ne 0 ]]; then
        exit 1
    fi
fi

PKG_CONFIG_PATH="${pwd}/dependencies;$PKG_CONFIG_PATH" cmake -DBOOST_ROOT:string=${pwd}/dependencies -DCEF_ROOT_DIR:string=${pwd}/dependencies/cef -DFreeImage_LIBRARY_SEARCH_DIRS:string=/usr/local/Cellar/freeimage/3.18.0/lib -G Xcode ../src || exit 1