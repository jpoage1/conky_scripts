-- 1. HELPER: Read the state file to determine mode (Local vs SSH)
local function get_current_mode()
    local f = io.open("/tmp/telemetry_mode", "r")
    if not f then return "local" end -- Default to local if file missing

    local content = f:read("*all")
    f:close()

    -- Trim whitespace
    content = content:gsub("^%s*(.-)%s*$", "%1")

    if content == "" then return "local" end
    return content
end
local function inherit(base)
    local orig_type = type(base)
    local copy
    if orig_type == 'table' then
        copy = {}
        for orig_key, orig_value in next, base, nil do
            copy[inherit(orig_key)] = inherit(orig_value)
        end
    else -- number, string, boolean, etc
        copy = base
    end
    return copy
end


local default_settings = {
    name = "Unknown System",
    features = {
        -- Core Features
        enable_sysinfo = true,
        enable_uptime = true,
        enable_memory = true,
        enable_cpu_temp = true,
        enable_cpuinfo = true,

        -- Stats
        enable_load_and_process_stats = true,
        enable_network_stats = true,
        enable_diskstat = true,
        processes = {
            enable_avg_cpu = true,
            enable_avg_mem = true,
            enable_realtime_cpu = true,
            enable_realtime_mem = true,

            -- How many processes to return?
            count = 10,

            -- Filter out specific process names?
            ignore_list = { "kworker", "rtkit-daemon" },
            only_user_processes = false,
        },
    },
    batteries = {
        {
            name = "Main",
            path = "/sys/class/power_supply/BAT0",
            critical_threshold = 15,
            icon = "⚡",
        }
    },
    -- [NETWORKING]
    network = {
        interfaces = {},
        -- Ping a target to check latency?
        ping_target = "8.8.8.8",
        enable_ping = false
    },

    -- [STORAGE]
    storage = {
        -- Filesystem devices
        filesystems = {},

        -- Specific devices to check IO speed for (sda, nvme0n1)
        -- If empty, might default to auto-detect
        io_devices = {},

        filters = {
            enable_loopback = false,
            enable_mapper = true,     -- Enable dm-0, etc
            enable_partitions = false -- Ignore sda1, sdb2...
        },
    },

    -- [SSH CONFIGURATION]
    ssh = {
        enabled = false, -- Default to false
        host = "",
        user = "",
        port = 22,
        key_path = "~/.ssh/id_rsa",

        -- Timeout in seconds before giving up
        timeout_sec = 5,
        -- Keepalive interval
        keepalive = true
    },
}


-- 3. CONDITIONAL CONFIGURATION
local local_settings = inherit(default_settings)

local_settings.name = "Local System"
local_settings.network.interfaces = {
    "eth0",
    "wlan0",
}

local_settings.storage.filesystems = {
    "/dev/path_to_filesystem",
}
local_settings.storage.io_devices = {
    "sda",
    "sdb",
}
local_settings.window.type = "desktop" -- desktop, dock, normal
local_settings.window.stacking = "bottom" -- fg, bottom
local_settings.window.wmIgnore = true -- true, false
local_settings.window.x = 20
local_settings.window.y = 30
local_settings.window.width = 800
local_settings.window.height = 950
local_settings.window.vertical_scroll = "on" -- on, off, auto
local_settings.window.horizontal_scroll = "on" -- on, off, auto
local_settings.window.visible = true -- true, false
local_settings.window.resizable = true -- true, false

-- === SSH MODE ===
local ssh_settings = inherit(default_settings)

ssh_settings.ssh = {
    enabled = true,
    host = "remote_addr",
    user = "conky",
    port = 22,
    key_path = "~/.ssh/id_rsa"
}

ssh_settings.name = string.format("%s@%s", ssh_settings.ssh.user, ssh_settings.ssh.host)

ssh_settings.network.interfaces = {
    "eth0", "wlan0"
}

ssh_settings.storage.filesystems = {
    "/dev/sda1",
    "/dev/sdb2",
}

ssh_settings.storage.io_devices = {
    "/dev/sda",
}


local mode = get_current_mode()

config = {
    -- App Behavior

    -- Options: "persistent", "run_once"
    run_mode = "persistent",

    -- Options: "json", "conky", "waybar", "qt", "gtk"
    output_format = "json",

    -- Unit: milliseconds
    polling_interval_ms = 1000,

    log_level = "warn", -- "debug", "info", "warning", "error"
    dump_to_file = false,
    log_file_path = "/tmp/telemetry_debug.log",

    settings = {
        local_settings,
        -- ssh_settings,
    }
}
return config
