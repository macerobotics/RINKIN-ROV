function mqtt.on_message(topic, payload)
    print("lua: mqtt: topic=" .. topic .. " payload=" ..payload)
end

function setup()
    local ip = "192.168.1.18"
    mqtt.connect(ip, 1883)
    mqtt.subscribe("/imu/heading")
    mqtt.subscribe("/imu/pitch")
    mqtt.subscribe("/imu/roll")
    v = video.new("rtsp://" .. ip .. ":8554/cam", 640, 480)
end

function loop()
    ImGui.Begin("Video")
    v:display()
    if ImGui.Button("Start") then v:start() end
    if ImGui.Button("Stop") then v:stop() end
    ImGui.End("Video")
end