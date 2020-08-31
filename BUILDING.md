# Building

To build mandarina from source, you need a C++11 compiler and cmake. Additionally, you need the following libraries

* [SFML 2.5.1](https://github.com/SFML/SFML)
* [rapidjson](https://github.com/Tencent/rapidjson) 
* [cpp-httplib](https://github.com/yhirose/cpp-httplib)
* GameNetworkingSockets

Any recent version of the first three libraries should work. For GameNetworkingSockets, it is recommended that you use [this version](https://github.com/ValveSoftware/GameNetworkingSockets/tree/89074ad22f882d4d8617e00ca5985e220665a9a6) (from December 2019).

## Linux

To build using CMakeLists.txt, you need to set the include/lib paths to the previous libraries. To do this, modify [CMakeLists.txt](CMakeLists.txt) to define them. For example, if I had installed SFML in `~/Libraries/SFML`, then I would have to write
```
set(PATH_SFML_include "~/Libraries/SFML/include")
set(PATH_SFML_lib     "~/Libraries/SFML/lib")
```
Note that both rapidjson and cpp-httplib don't need to be built, so only their include variable has to be set. Once all dependencies are installed, the building process is straightforward.

```
mkdir build_debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

You can of course set `-DCMAKE_BUILD_TYPE=Release` for release mode. Note that `MANDARINA_DEBUG` is defined on debug mode. To test that the executable works simply run it like so:
```
./mandarina --local-connection
```

You can run the game in client/server mode doing
```
./mandarina --client
./mandarina --server
```
This of course requires additional setup.

## Other platforms

I don't have time to test the building process on other platforms, but it should work as long as the libraries are installed.
