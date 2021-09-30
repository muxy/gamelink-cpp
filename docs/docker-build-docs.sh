#!/bin/bash
rm -rf build doxygen/html doxygen/latex doxygen/xml && mkdir -p build && cd build && cmake .. && make doxygen && make sphinx && tar --overwrite -czvf ../docs.tar.gz ../docs/sphinx/*