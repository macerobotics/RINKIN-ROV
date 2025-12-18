-- commandes moteurs :
-- #0m20!   <- moteur 0, vitesse 20
-- moteur de 0 à 4, vitesse de -20 à 20

video_resolution = {x = 640, y = 480}
speed = 0
gamepad_enabled = false

function setup()
    local ip = "192.168.0.1"
    v = video.new("rtsp://" .. ip .. ":8554/cam", video_resolution.x, video_resolution.y)
    udp.init(ip, 1234)
    heading = plot.new("heading", 1000)
    pitch = plot.new("pitch", 1000)
    roll = plot.new("roll", 1000)
    speed_plot = plot.new("vitesse", 1000)
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
    if gamepad_enabled then
     speed = round(-20 * gamepad.get_axis_movement(0, 3))
    end

    speed_plot:append(speed)

    cmd = "#0m" .. tostring(math.floor(speed)) .. "!\n"
    --udp.send(cmd)

    if gamepad.is_button_pressed(0, 11) then
        udp.send("#LEDB,1!\n")
    elseif gamepad.is_button_released(0, 11) then
        udp.send("#LEDB,0!\n")
    end

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
        if ImGui.Button("Allumer LED") then
            udp.send("#LEDB,1!\n")
        end
        ImGui.SameLine()
        if ImGui.Button("Éteindre LED") then
            udp.send("#LEDB,0!\n")
        end
        gamepad_enabled = ImGui.Checkbox("Gamepad activé", gamepad_enabled)
        local modified
        speed, modified = ImGui.SliderInt("Vitesse moteur", speed, -20, 20)
        if modified then
            udp.send("#0m" .. tostring(speed) .. "!\n")
        end
        if plot.Begin("Vitesse moteur##plot", 320, 240) then
            plot.x_axis_limits(0, 1000, "always")
            plot.y_axis_limits(-20, 20, "always")
            speed_plot:display()
            plot.End()
        end

        if ImGui.BeginChild("Script", 100, 100) then
            if ImGui.Button("Recharger") then dofile("lua/main.lua") end
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

function round(x)
    if x >= 0 then
        return math.floor(x + 0.5)
    else
        return math.ceil(x + 0.5)
    end
end