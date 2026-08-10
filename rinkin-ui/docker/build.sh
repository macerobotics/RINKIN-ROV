#!/bin/bash
set -euxo pipefail

pushd ../../..
pwd
ls
docker build -t rinkin -f RINKIN-ROV/rinkin-ui/docker/Dockerfile .
docker create --name tmp rinkin
docker cp tmp:/work/RINKIN-ROV/rinkin-ui/output/rinkin-linux-x86_64 RINKIN-ROV/rinkin-ui/docker
docker cp tmp:/work/RINKIN-ROV/rinkin-ui/output/rinkin.exe RINKIN-ROV/rinkin-ui/docker
docker cp tmp:/work/RINKIN-ROV/rinkin-ui/output/rinkin-raspberry-pi RINKIN-ROV/rinkin-ui/docker
docker rm tmp

popd
