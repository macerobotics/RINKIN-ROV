#!/bin/bash
set -euxo pipefail

IP=192.168.1.18

if [ ! -f setup/mediamtx/mediamtx ]; then
    pushd setup/mediamtx
    wget https://github.com/bluenviron/mediamtx/releases/download/v1.15.1/mediamtx_v1.15.1_linux_arm64.tar.gz
    tar -xvf mediamtx_v1.15.1_linux_arm64.tar.gz mediamtx
    rm mediamtx_v1.15.1_linux_arm64.tar.gz
    popd
fi

rsync -avh setup rinkin@${IP}:~
rsync -avh rinkin rinkin@${IP}:~
ssh rinkin@${IP} "cd setup && bash setup.sh"