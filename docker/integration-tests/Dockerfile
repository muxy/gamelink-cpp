FROM teeks99/clang-ubuntu
RUN apt-get update
RUN apt-get -y install cmake ninja-build libwebsockets-dev libcurl4-openssl-dev pkg-config

ADD . /gamelink-cpp
WORKDIR gamelink-cpp

RUN rm -rf build && mkdir -p build && cd build && cmake .. -GNinja && cd ..
RUN cmake --build build --target tests -- -j8

ENTRYPOINT ["./build/tests"]