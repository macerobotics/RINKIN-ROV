video_resolution = {x = 640, y = 480}

function setup()
    local ip = "192.168.10.90"
    v = video.new("rtsp://" .. ip .. ":8554/cam", video_resolution.x, video_resolution.y)
    udp.init(ip, 1234)
    heading = plot.new("heading", 1000)
    pitch = plot.new("pitch", 1000)
    roll = plot.new("roll", 1000)
    udp.on_receive = function(msg)
        print("udp: " .. msg)
        local command, param = msg:match("^#(%a+),([^!]+)!$")
        param = tonumber(param)
        print(command, param)
        if command == "heading" then
            model.set_heading(param)
            heading:append(param)
        elseif command == "pitch" then
            model.set_pitch(param)
            pitch:append(param)
        elseif command == "roll" then
            model.set_roll(param)
            roll:append(param)
        end
    end

end

function loop()
    --[[
    if gamepad.is_button_pressed(0, 11) then
        udp.send("#MOT 0.1\n")
    elseif gamepad.is_button_released(0, 11) then
        udp.send("#MOT 0\n")
    end
    --]]
    udp.send("#MOT " .. tostring(-1 * gamepad.get_axis_movement(0, 3)) .. "\n")

    ImGui.SetNextWindowPos(0, 0)
    ImGui.SetNextWindowSize(ImGui.GetViewportSize())
    if ImGui.Begin("Rinkin") then
        ImGui.BeginGroup("Video")
            ImGui.Text("Vidéo")
            v:display()
            if ImGui.Button("Démarrer") then v:start() end
            ImGui.SameLine()
            if ImGui.Button("Arrêter") then v:stop() end
        ImGui.EndGroup()
        
        ImGui.SameLine()
        ImGui.BeginGroup()
            ImGui.Text("Modèle 3D")
            model.display()
        ImGui.EndGroup()

        ImGui.SeparatorText("Script")

        if ImGui.BeginChild("Script", 100, 100) then
            if ImGui.Button("Recharger") then dofile("lua/main.lua") end
            if ImGui.Button("Envoyer") then udp.send("Hello, World!") end
            ImGui.Text(tostring(gamepad.get_axis_count(0)))
        end
        ImGui.EndChild()

        ImGui.SeparatorText("Gamepad")

        if ImGui.BeginChild("foo") then
            ImGui.Text(tostring(gamepad.is_button_down(0, 11)))
        end
        ImGui.EndChild()

        if plot.Begin("IMU", 640, 400) then
            plot.x_axis_limits(0, 1000, "always")
            plot.y_axis_limits(0, 360)
            heading:display()
            pitch:display()
            roll:display()
            plot.End()
        end
        
    end
    ImGui.End()
end