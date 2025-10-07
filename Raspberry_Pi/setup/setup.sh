#!/bin/bash
set -euxo pipefail

# Enable i2c + uart
sudo raspi-config nonint do_i2c 0
sudo raspi-config nonint do_serial_hw 0
sudo raspi-config nonint do_serial_cons 1

sudo apt update
sudo apt install mosquitto python3-pip -y

cd /home/rinkin/setup 
sudo cp -u mosquitto.conf /etc/mosquitto/conf.d/
sudo systemctl restart mosquitto

cd /home/rinkin/rinkin
python3 -m venv .venv
ls .venv/bin
.venv/bin/pip install -r requirements.txt
sudo cp -u rinkin.service /etc/systemd/system
sudo systemctl daemon-reload
sudo systemctl enable rinkin.service
sudo systemctl start rinkin.service

