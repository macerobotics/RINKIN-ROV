#!/bin/bash
set -euxo pipefail

docker build -t rinkin .
docker create --name tmp rinkin
docker cp tmp:/work/RINKIN-ROV/rinkin-ui/output/rinkin-linux-x86_64 .
docker cp tmp:/work/RINKIN-ROV/rinkin-ui/output/rinkin.exe .
docker rm tmp
