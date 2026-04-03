-- commandes moteurs :
-- #0m20!   <- moteur 0, vitesse 20
-- moteur de 0 à 4, vitesse de -9 à 9

local motor = require("motor")

video_resolution = {x = 640, y = 480}


motors = {}
for i = 1, 5 do
    table.insert(motors, motor:new(i))
end


gamepad_enabled = true

pow = 1.0

local function axis(i)
    local val = gamepad.get_axis_movement(0, i)
    if i == 4 or i == 5 then
        val = (val + 1) / 2
    end
    if val >= 0 then
        return val ^ pow
    else
        return - math.abs(val) ^ pow
    end
end

function setup()
    --local ip = "192.168.0.1"
    --local ip = "192.168.10.90"
    local ip = "192.168.4.1"
    v = video.new("rtsp://" .. ip .. ":8554/cam", video_resolution.x, video_resolution.y)
    udp.init(ip, 1234)
    heading = plot.new("heading", 1000)
    pitch = plot.new("pitch", 1000)
    roll = plot.new("roll", 1000)
    udp.on_receive = function(msg)
        --print("udp: " .. msg)
        local command, param = msg:match("^#(%a+),([^!]+)!%s*$")
        param = tonumber(param)
        --print(command, param)
        if command == "heading" then
            model.set_heading(param)
            heading:append(param)
        elseif command == "pitch" then
            model.set_pitch(param)
            pitch:append(param)
        elseif command == "roll" then
            model.set_roll(param)
            roll:append(param)
        elseif command == "BAT" then
        	print("batterie:", param)
        end
    end

end

function loop()
    if gamepad_enabled then
     --motors[1]:set_speed(round(-20 * gamepad.get_axis_movement(0, 3)))
    end

    for _, m in ipairs(motors) do
        m:update()
    end


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
        if ImGui.Button("Allumer LED A") then
            udp.send("#0l1!\n")
        end
        ImGui.SameLine()
        if ImGui.Button("Éteindre LED A") then
            udp.send("#0l0!\n")
        end
        ImGui.SameLine()
        if ImGui.Button("Allumer LED B") then
            udp.send("#1l1!\n")
        end
        ImGui.SameLine()
        if ImGui.Button("Éteindre LED B") then
            udp.send("#1l0!\n")
        end
        ImGui.SameLine()
        if ImGui.Button("Allumer Test") then
            udp.send("#2l1!\n")
        end
        ImGui.SameLine()
        if ImGui.Button("Eteindre Test") then
            udp.send("#2l0!\n")
        end
        ImGui.SameLine()
        if ImGui.Button("Batterie") then
            udp.send("#0b0!\n")
        end

        gamepad_enabled = ImGui.Checkbox("Gamepad activé", gamepad_enabled)

        pow = ImGui.SliderFloat("pow", pow, 0, 5)

        if(gamepad_enabled) then
            motors[1]:set_speed(round((axis(1) + axis(4) - axis(5)) * 9))
            motors[2]:set_speed(round((-axis(3) + axis(0)) * 9))
            motors[3]:set_speed(round((-axis(3) - axis(0)) * 9))
            motors[4]:set_speed(round((-axis(1) + axis(4) - axis(5)) * 9))
            motors[5]:set_speed(round((-axis(1) + axis(4) - axis(5)) * 9))
        end

        for _, m in ipairs(motors) do
            m:slider()
        end

        if plot.Begin("Vitesse moteur##plot", 320, 240) then
            plot.x_axis_limits(0, 1000, "always")
            plot.y_axis_limits(-9, 9, "always")
            for _, m in ipairs(motors) do
                m:display_plot()
            end
            plot.End()
        end
--[[
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
]]--
        ImGui.SameLine()
        if plot.Begin("IMU", 320, 240) then
            plot.x_axis_limits(0, 1000, "always")
            plot.y_axis_limits(0, 360)
            heading:display()
            pitch:display()
            roll:display()
            plot.End()
        end

        ImGui.SameLine()

        if gamepad.is_available(0) then
            if ImGui.BeginChild("Gamepad", 320, 240) then
                ImGui.Text(gamepad.get_name(0))
                local axis_count = gamepad.get_axis_count(0)
                for i = 0, axis_count - 1 do
                    ImGui.PushID(i)
                    --local val = gamepad.get_axis_movement(0, i)
                    local val = axis(i)
                    ImGui.InputDouble(tostring(i), val)
                    ImGui.PopID();
                end
            end
            ImGui.EndChild()
        end
        
    end
    ImGui.End()
end

function round(x)
    if x >= 0 then
        x = math.floor(x + 0.5)
    else
        x = math.ceil(x + 0.5)
    end
    return math.tointeger(x)
end
