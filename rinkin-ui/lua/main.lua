function setup()
    local ip = "192.168.1.18"
    v = video.new("rtsp://" .. ip .. ":8554/cam", 640, 480)
    udp.init(ip, 1234)
    udp.on_receive = function(msg)
        print("udp: " .. msg)
        local command, param = msg:match("^#(%a+),([^!]+)!$")
        print(command, param)
        if command == "heading" then
            model.set_heading(tonumber(param))
        elseif command == "pitch" then
            model.set_pitch(tonumber(param))
        elseif command == "roll" then
            model.set_roll(tonumber(param))
        end
    end
end

function loop()
    if ImGui.Begin("Video") then
        v:display()
        if ImGui.Button("Start") then v:start() end
        ImGui.SameLine()
        if ImGui.Button("Stop") then v:stop() end
    end
    ImGui.End("Video")
    if ImGui.Begin("Script") then
        if ImGui.Button("Reload") then dofile("lua/main.lua") end
        if ImGui.Button("send") then udp.send("Hello, World!") end
        ImGui.Text(tostring(gamepad.get_axis_count(0)))
    end
    ImGui.End()
end