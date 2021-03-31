### This dockerfile only runs and builds the tests, which 
### should be platform-independent.
FROM ubuntu:latest
RUN apt-get -y update && apt-get install -y
RUN apt-get -y install clang cmake ninja-build

COPY . /gamelink
WORKDIR gamelink

RUN rm -rf build && mkdir -p build && cd build && \
    cmake .. -GNinja && cmake --build . --target tests &&\
    cmake --build . --target amalgam \
    && cd .. && ./amalgamate.sh \
    && cmake --build build --target amalgam_compile_check

CMD ["./dockerentry.sh"]