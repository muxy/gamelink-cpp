## Schema Tests
To run the schema tests:

  1. create a build directory and CD into it: `mkdir build && cd build`
  2. Generate the cmake build system in the build directory: `cmake ..`
  3. Build the binary: `cmake --build .`
  4. Run the tests: `./schema_tests`

## Docs
To build the documentation

  1. Ensure that you have doxygen and sphinx installed.
  2. Run cmake, and then run `make Doxygen && make Sphinx`
  3. Distribute the docs/sphinx directory