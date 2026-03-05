local motor = {}

function motor:new(n)
    local m = {
        n = n,
        speed = 0,
        plot = plot.new("moteur " .. tostring(n), 1000)
    }
    return setmetatable(m, {__index = self})
end

function motor:set_speed(s)
    self.speed = s
end

function motor:send_speed()
    udp.send("#" .. tostring(self.n - 1) .." m" .. tostring(self.speed) .. "!\n")
end

function motor:update()
    self.plot:append(self.speed)
end

function motor:slider()
    local modified
    self.speed, modified = ImGui.SliderInt("Vitesse moteur " .. tostring(self.n), self.speed, -20, 20)
    if modified then
        self:send_speed()
    end
end

function motor:display_plot()
    self.plot:display()
end

return motor