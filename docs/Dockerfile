from teeks99/clang-ubuntu

WORKDIR /data

ENV DEBIAN_FRONTEND=noninteractive 
RUN apt-get update && apt-get -y install doxygen python3-sphinx cmake pkg-config python3-pip && pip3 install breathe sphinx-rtd-theme

ENTRYPOINT ["/data/docs/docker-build-docs.sh"]