
function network_line(dev)
    local up = string.format("${upspeed %s}", dev)
    local down = string.format("${downspeed %s}", dev)
    return string.format("${color lightgrey}%s  ${goto 120}%s${goto 250}%s", dev, up, down)
end

function network_graph(dev)
    local upgraph = string.format("${upspeedgraph %s 20,150}", dev)
    local downgraph = string.format("${downspeedgraph %s 20,150}", dev)
    return string.format("%s %s", upgraph, downgraph)
end

function conky_network_graph(dev)
    return conky_parse(network_graph(dev))
end

function conky_network_line(dev)
    return conky_parse(network_line(dev))
end

function conky_network_info(dev)
    return conky_parse(network_line(dev) .. "\n" .. network_graph(dev))
end
-- function conky_network_info(dev)
--     network_line(dev)
--     network_graph(dev)
-- end
function conky_network_table(...)
    local devices = {...}
    local text = ""
    local base_goto = 100
    local step = 170
    local graph_width = 20
    local graph_height = 150

    -- Header line: Device names
    text = text .. "${color 00ccff}Device:"
    for i, dev in ipairs(devices) do
        local goto_pos = base_goto + (i - 1) * step
        text = text .. string.format("${goto %d}${color lightgrey}%-10s", goto_pos, dev)
    end
    text = text .. "\n"

    -- Up speeds
    text = text .. "${color 00ccff}Up:"
    for i, dev in ipairs(devices) do
        local goto_pos = base_goto + (i - 1) * step
        text = text .. string.format("${goto %d}${upspeed %s}", goto_pos, dev)
    end
    text = text .. "\n"

    -- Up graphs
    text = text .. "${color 00ccff}Graph:"
    for i, dev in ipairs(devices) do
        local goto_pos = base_goto + (i - 1) * step
        text = text .. string.format("${goto %d}${upspeedgraph %s %d,%d ffffff ffffff 0 -l -t}", goto_pos, dev, graph_width, graph_height)
    end
    text = text .. "\n"

    -- Down speeds
    text = text .. "${color 00ccff}Down:"
    for i, dev in ipairs(devices) do
        local goto_pos = base_goto + (i - 1) * step
        text = text .. string.format("${goto %d}${downspeed %s}", goto_pos, dev)
    end
    text = text .. "\n"

    -- Down graphs
    text = text .. "${color 00ccff}Graph:"
    for i, dev in ipairs(devices) do
        local goto_pos = base_goto + (i - 1) * step
        text = text .. string.format("${goto %d}${downspeedgraph %s %d,%d 00ccff ff0000 0 -l -t}", goto_pos, dev, graph_width, graph_height)
    end
    text = text .. "\n"

    return conky_parse(text)
end
