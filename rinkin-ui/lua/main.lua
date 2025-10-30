function setup()
    local ip = "192.168.1.18"
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