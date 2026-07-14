#!/bin/bash
set -euxo pipefail

#pushd ../..
#pwd
#ls
#docker build -f rinkin-ui/docker/Dockerfile --output type=local,dest=./rinkin-ui/docker/out .
docker build --output type=local,dest=./out .