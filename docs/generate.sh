#!/bin/bash

## Run this from the root gamelink-cpp directory
docker build -t gamelink-docs docs/
docker run -v $(pwd):/data gamelink-docs