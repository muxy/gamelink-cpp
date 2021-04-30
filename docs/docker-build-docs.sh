#!/bin/bash
rm -rf build && mkdir -p build && cd build && cmake .. && make doxygen && make sphinx && tar --overwrite -czvf ../docs.tar.gz ../docs/sphinx/*