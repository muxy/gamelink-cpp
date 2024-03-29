# GameLink

This repo contains a C++ library that provides access to Muxy’s GameLink API.
The library is distributed as a single-file, header-only library for ease of integration.

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

If libcurl and libwebsockets are availiable, the tests suite will also build
the integration tests. To run the integration tests, generate a valid JWT and
run the tests binary with the environment variables MUXY_INTEGRATION_JWT and
MUXY_INTEGRATION_ID with the test filter "[integration]"

### Building Test Suite

You must install vcpkg first:
[https://github.com/microsoft/vcpkg#quick-start-unix].

The test runner can then be compiled using CMake:

```sh
rm -rf build \
  && mkdir -p build \
  && cmake \
      -B ./build \
      -S . \
      "-DCMAKE_TOOLCHAIN_FILE=<path to vcpkg>/scripts/buildsystems/vcpkg.cmake" \
      --preset tests \
  && cmake --build ./build
  && ln -s tests build/Debug/tests
```

You can then run the tests: `./tests`
