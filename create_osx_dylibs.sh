cmake . --preset=x64-library   && cmake --build . --preset=x64-cgamelink
cmake . --preset=arm-library   && cmake --build . --preset=arm-cgamelink

lipo -create builds/x64-library/Release/libcgamelink.dylib builds/arm-library/Release/libcgamelink.dylib -output libcgamelink.dylib