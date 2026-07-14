# GameLink

This repo contains a C++ library that provides access to Muxy’s GameLink API.
The library is distributed as a single-file, header-only library for ease of integration.

The supported release line is `0.2.x`. Release artifacts include the generated single header and
the `cgamelink` C ABI library for Windows, macOS, and Linux.

## Integrating the library

The repo contains a pre-compiled version of the single header library in the root
of the project, `gamelink_single.hpp`. You may also compile the library from
source by using the CMake project target `amalgam` to generate the library
header file.

## Documentation

The library is documented using in-line Doxygen-style comments in
`include/gamelink.h`. A distributable form of the documentation can be built
using Doxygen and Sphinx.

## Configuration

Near the top of `gamelink_single.hpp` there are two configuration macros,
`MUXY_GAMELINK_CUSTOM_STRING_TYPE` and `MUXY_GAMELINK_CUSTOM_LOCK_TYPE` that can
be defined to modify the string and lock types used by the C++ library. The
inline documentation describes the required methods and behaviors that the string
and lock types must satisfy.

## Testing

The test suite can be built through the CMake target `tests`.

If libcurl is available, you can opt into the network integration tests. To run them, generate a valid JWT and
run the tests binary with the environment variables MUXY_INTEGRATION_JWT and
MUXY_INTEGRATION_ID with the test filter "[integration]"

### Building Test Suite

Configure and build the verified public surfaces with CMake:

```sh
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake \
  -DGAMELINK_BUILD_TESTS=ON \
  -DGAMELINK_BUILD_EXAMPLES=ON
cmake --build build --target \
  gamelink_tests amalgam_compile_check amalgam_drift cgamelink_compile_check examples
ctest --test-dir build --output-on-failure
```

The current protocol and native API guides are published at
[docs.muxy.io](https://docs.muxy.io/reference/gamelink-library/).
