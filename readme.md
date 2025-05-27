# 2025 Spring 3S综合实习 校园三维无人机模拟平台

**Author:** 万科君 童川博 张佳俊 邱馨 王芊卓

## Install

### Dependencies

At first, you need to install the dependencies.

- QGIS3
- Qt 5.15.x

Then clone the repository into your local machine.

``` bash
git clone https://github.com/OSWGeo4W/3Dschool_app.git
cd 3Dschool_app
```

### Install as a standalone application

``` bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --parallel
```

the executable will be output to `build/app/bin/3Dschool_app`

**If you want to build a release version, you can use the following command:**

``` bash
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel
```

This will build a release version of the application for distribution.

### Install as a QGIS plugin
``` bash
mkdir build
cd build
cmake -DBUILD_AS_PLUGIN=ON ..
cmake --build . --parallel --config release
```
the plugin will be output to `build/src/3Dschool_app.so(3Dschool_app.dll for Windows)

## Setup

### Use as a standalone application
The first time you run the application, you need to execute `run.sh` in a shell to setup the environment.

### Use as a QGIS plugin

then copy the plugin to the OSWGeo4W QGIS plugin directory On Windows, it is `C:\OSGeo4W\apps\qgis\plugins`
and on Linux, it is `~/.local/share/QGIS/QGIS3/profiles/default/python/plugins`


