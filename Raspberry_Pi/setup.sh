#!/bin/bash
set -euxo pipefail

IP=192.168.1.18

rsync -avh setup rinkin@${IP}:~
rsync -avh rinkin rinkin@${IP}:~
ssh rinkin@${IP} "cd setup && bash setup.sh"