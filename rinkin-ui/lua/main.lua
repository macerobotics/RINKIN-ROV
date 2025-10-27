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
    if ImGui.Begin("Video") then
        v:display()
        if ImGui.Button("Start") then v:start() end
        if ImGui.Button("Stop") then v:stop() end
    end
    ImGui.End("Video")
    if ImGui.Begin("Script") then
        if ImGui.Button("Reload") then dofile("lua/main.lua") end
    end
    ImGui.End()
end