#!/bin/bash
rm -rf build && mkdir -p build && cd build && cmake .. && make Doxygen && make Sphinx && tar --overwrite -cvf ../docs.tar.gz ../docs/sphinx/*