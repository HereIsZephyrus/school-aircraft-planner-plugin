# 2025 Spring 3S综合实习QGIS插件

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
cmake ..
cmake --build . --parallel --config release
```

the executable will be output to `build/app/bin/3Dschool_app`

### Install as a QGIS plugin
``` bash
mkdir build
cd build
cmake .. -DBUILD_AS_PLUGIN=ON
cmake --build . --parallel --config release
```

## Setup

### Use as a standalone application
The first time you run the application, you need to execute `run.sh` in a shell to setup the environment.

### Use as a QGIS plugin
the plugin will be output to `build/src/3Dschool_app.so(3Dschool_app.dll for Windows)

then copy the plugin to the OSWGeo4W QGIS plugin directory On Windows, it is `C:\OSGeo4W\apps\qgis\plugins`
and on Linux, it is `~/.local/share/QGIS/QGIS3/profiles/default/python/plugins`


