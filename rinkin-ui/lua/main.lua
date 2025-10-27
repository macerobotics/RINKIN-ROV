ip = "192.168.1.18"
port = 1883
mqtt.connect(ip, port)
mqtt.subscribe("/imu/heading")

function mqtt.on_message(topic, payload)
    print("lua: mqtt: topic=" .. topic .. " payload=" ..payload)
end