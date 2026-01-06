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

local mode = get_current_mode()

-- 2. BASE SETTINGS
settings = {
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
            ignore_list = { "kworker", "rtkit-daemon" }
        },
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
if mode == "local" then
    settings.name = "Local System"
    settings.network.interfaces = {
        "eth0",
        "wlan0",
    }

    settings.storage.filesystems = {
        "/dev/path/to/filesystem/partition",
    }
    settings.storage.io_devices = {
        "sda",
        "sdb",
    }
else
    -- === SSH MODE ===

    settings.ssh = {
        enabled = true,
        host = "remote_addr",
        user = "telemetry",
        port = 22,
        key_path = "~/.ssh/id_rsa"
    }

    settings.name = string.format("%s@%s", settings.ssh.user, settings.ssh.host)

    settings.network.interfaces = {
        "eth0",
        "wlan0",
    }

    settings.storage.filesystems = {
        "/dev/path/to/filesystem/partition",
    }

    settings.storage.io_devices = {
        "/dev/path/to/disk",
    }
end

-- Return the table so C++ can read it
return settings
