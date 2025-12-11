// parser.cpp
#include "parser.hpp"

#include "conky_output.hpp"
#include "data_local.hpp"
#include "data_ssh.hpp"
#include "json_serializer.hpp"
#include "log.hpp"
#include "polling.hpp"
#include "runner.hpp"

//
#include "json_definitions.hpp"

ParsedConfig parse_arguments(int argc, char* argv[]) {
  ParsedConfig config;
  const char* prog_name = argv[0];  // Get program name

  if (argc < 2) {
    std::cerr << "Error: No arguments provided." << std::endl;
    print_usage(prog_name);
    return config;  // Return empty
  }

  std::vector<std::string> args_in(argv + 1,
                                   argv + argc);  // args_in is argv[1] onwards

  // ---------------------------------------------------------
  // PRIORITY CHECK: --config (Exclusive Mode)
  // ---------------------------------------------------------
  // If --config is present, we ignore all other flags and load only that file.

  for (size_t i = 0; i < args_in.size(); ++i) {
    if (args_in[i] == "--config") {
      if (i + 1 < args_in.size()) {
        // 1. Check for duplicates
        int config_count = 0;
        for (const auto& arg : args_in) {
          if (arg == "--config") config_count++;
        }

        if (config_count > 1) {
          std::cerr << "Warning: Multiple --config flags detected. Only the "
                       "first one is being consumed."
                    << std::endl;
        }
        // 2. Check for other ignored flags (e.g. --local mixed with --config)
        else if (args_in.size() > 2) {
          std::cerr << "Warning: --config flag detected. All other flags are "
                       "being ignored."
                    << std::endl;
        }

        std::string filename = args_in[i + 1];
        std::cerr << "Loading global config: " << filename << std::endl;

        config = load_lua_config(filename);

        // Stop parsing immediately
        return config;
      } else {
        std::cerr << "Error: --config requires a filename." << std::endl;
        return config;  // Return empty/invalid
      }
    }
  }

  // ---------------------------------------------------------
  // CLI MODE
  // ---------------------------------------------------------
  // If we reach here, no --config was passed. Parse other flags normally.
  std::vector<std::string> args_full;
  args_full.push_back(prog_name);  // argv[0] equivalent
  args_full.insert(args_full.end(), args_in.begin(), args_in.end());

  size_t i = 1;  // Index for args_full (starts at argv[1] equivalent)

  while (i < args_full.size()) {
    std::string command = args_full[i];
    int consumed = 0;

    // Handle Implicit Local
    if (i == 1 && command.rfind("--", 0) != 0) {
      std::vector<std::string> temp_args = {prog_name, "--local"};
      temp_args.insert(temp_args.end(), args_in.begin(), args_in.end());
      size_t temp_i = 1;
      consumed = process_command(temp_args, temp_i, config.tasks);
      consumed = (consumed > 0) ? consumed - 1 : 0;
      i += (consumed > 0) ? consumed : 1;
    }
    // Handle Persistent Flag
    else if (command == "--persistent") {
      config.set_run_mode(RunMode::PERSISTENT);
      i++;  // Consume this flag and continue parsing
    }  // Handle Explicit Commands
    else if (command == "--local" || command == "--ssh" ||
             command == "--settings") {
      consumed = process_command(args_full, i, config.tasks);
      // Ensure loop advances if process_command fails/returns 0 or doesn't
      // update 'i'
      if (consumed == 0) {
        i++;  // Prevent infinite loop
      }
      // If process_command updates 'i' by reference, this 'else' block isn't
      // needed else {
      //    i += consumed;
      // }
    }

    // Handle Unknown
    else {
      MetricsContext context;
      context.source_name = "Parser";
      context.success = false;
      context.error_message = "Unknown command or flag: " + command;
      //   config.tasks.push_back(context);
      config.tasks.push_back(std::move(context));
      i++;  // Consume unknown command
    }
  }  // end while

  // Final checks
  if (config.tasks.empty() && config.run_mode(RUN_ONCE)) {
    std::cerr << "Error: No valid commands contexted in metrics." << std::endl;
  } else {
    // bool any_success = false;
    // for (const auto& res : config.tasks) {
    //   if (res.success) {
    //     any_success = true;
    //     break;
    //   }
    // }
    // if (!any_success) {
    //   std::cerr << "Warning: All processed commands contexted in errors."
    //             << std::endl;
    // }
  }

  return config;
}

int check_config_file(const std::string& config_file) {
  if (!std::filesystem::exists(config_file)) {
    std::cerr << "Warning: Config file not found, skipping: " << config_file
              << std::endl;
    return 1;
  }
  return 0;
}

void print_usage(const char* prog_name) {
  std::cerr
      << "Usage: " << prog_name << " [options...]\n\n"
      << "Generates metrics based on one or more commands.\n"
      << "If the first argument is a file path, it defaults to --local.\n\n"
      << "Commands:\n"
      << "  <config_file>       (As first argument) Generate local metrics.\n"
      << "  --local <config_file>\n"
      << "                      Generate local metrics.\n"
      << "  --ssh <config_file>\n"
      << "                      Generate metrics from default SSH host.\n"
      << "  --ssh <config_file> <host> <user>\n"
      << "                      Generate metrics from specific SSH host.\n\n"
      << "Example:\n"
      << "  " << prog_name
      << " /path/local.conf --ssh /path/ssh.conf my-server conky\n";
}

std::set<std::string> parse_interface_list(const std::string& list_str) {
  std::set<std::string> interfaces;
  std::stringstream ss(list_str);
  std::string interface_name;
  while (std::getline(ss, interface_name, ',')) {
    if (!interface_name.empty()) {
      interfaces.insert(interface_name);
    }
  }
  return interfaces;
}
int process_command(const std::vector<std::string>& args, size_t& current_index,
                    std::vector<MetricsContext>& tasks) {
  MetricsContext context;
  std::string command = args[current_index];
  SPDLOG_DEBUG("Processing command: {}", command);
  size_t initial_index = current_index;

  // --- 1. Parse Config File ---
  if (current_index + 1 >= args.size()) {
    context.success = false;
    context.error_message = command + " requires a <config_file> argument.";
    context.source_name =
        (command == "--local") ? "Local (Error)" : "SSH (Error)";
    tasks.push_back(std::move(context));
    return 1;  // Consume only the command itself
  }
  std::string config_file = args[current_index + 1];
  current_index += 2;  // Tentatively consume command + config

  // --- 2. Check Config File ---
  if (check_config_file(config_file) != 0) {
    context.success = false;
    context.error_message = "Config file not found: " + config_file;
    context.source_name =
        (command == "--local") ? "Local (Error)" : "SSH (Error)";
    tasks.push_back(std::move(context));
    return current_index - initial_index;  // Return consumed args
  } else if (command == "--settings") {
    std::cerr << "Loading lua config" << std::endl;
    context = load_lua_settings(config_file);
    tasks.push_back(std::move(context));
    return current_index - initial_index;
  } else {
    context.device_file = config_file;
  }

  // --- 3. Call Appropriate Get Function ---
  if (command == "--local") {
    context.source_name = "Local";
    context.provider = DataStreamProviders::LocalDataStream;
  } else if (command == "--ssh") {
    // Check for specific host/user
    if (current_index + 1 < args.size() &&
        args[current_index].rfind("--", 0) != 0 &&
        args[current_index + 1].rfind("--", 0) != 0) {
      std::string host = args[current_index];
      std::string user = args[current_index + 1];
      context.source_name = user + "@" + host;

      context.provider = DataStreamProviders::ProcDataStream;
      context.host = host;
      context.user = user;
      current_index += 2;  // Consume host + user
    } else {
      // Default SSH host
      context.source_name = "Default SSH";
      context.provider = DataStreamProviders::ProcDataStream;
    }
  }
  // --- 4. Check for --interfaces ---
  if (current_index < args.size() && args[current_index] == "--interfaces") {
    if (current_index + 1 < args.size()) {
      context.interfaces = parse_interface_list(args[current_index + 1]);
      current_index += 2;  // Consume --interfaces and its value
    } else {
      // Handle error: --interfaces flag without a value
      // You might want to add an error to context.error_message
      std::cerr << "Warning: --interfaces flag requires a comma-separated list."
                << std::endl;
      current_index += 1;  // Consume only the flag to avoid infinite loop
    }
  }

  tasks.push_back(std::move(context));
  return current_index - initial_index;  // Return total consumed args
}

bool ParsedConfig::run_mode(RunMode mode) const { return _run_mode == mode; }
bool ParsedConfig::output_mode(OutputMode mode) const {
  return _output_mode == mode;
}
RunMode ParsedConfig::run_mode() const { return _run_mode; }
OutputMode ParsedConfig::get_output_mode() const { return _output_mode; }
void ParsedConfig::set_run_mode(RunMode mode) { _run_mode = mode; }
void ParsedConfig::set_output_mode(OutputMode mode) { _output_mode = mode; }

void ParsedConfig::set_output_mode(std::string mode) {
  if (mode == "json") {
    _output_mode = OutputMode::JSON;
  } else if (mode == "conky") {
    _output_mode = OutputMode::CONKY;
  } else {
    std ::cerr << "Error: invalid output mode `" << mode << "`" << std::endl;
  }
}
void ParsedConfig::set_run_mode(std::string mode) {
  if (mode == "persistent") {
    _run_mode = RunMode::PERSISTENT;
  } else if (mode == "run_once") {
    _run_mode = RunMode::RUN_ONCE;
  } else {
    std ::cerr << "Error: invalid run mode `" << mode << "`" << std::endl;
  }
}
void ParsedConfig::sleep() {
  sleep_until += get_polling_interval<std::chrono::milliseconds>();
  std::this_thread::sleep_until(sleep_until);
}

void ParsedConfig::configure_renderer() {
  for (MetricsContext& task : tasks)
    // 1. Select and Configure the pipeline ONCE
    switch (_output_mode) {
      case OutputMode::JSON: {
        this->active_pipeline = configure_json_pipeline(task.settings);
        break;
      }
      case OutputMode::CONKY: {
        this->active_pipeline = configure_conky_pipeline(task.settings);
        break;
      }
      default:
        std::cerr << "Invalid output type" << std::endl;
        exit(1);
    }
}

int ParsedConfig::initialize(std::list<SystemMetrics>& tasks) {
  this->configure_renderer();

  if (this->tasks.empty()) {
    std::cerr << "Initialization failed, no valid tasks to run." << std::endl;
    return 1;
  }
  /* Perform these steps only once */
  for (MetricsContext& task : this->tasks) {
    SystemMetrics& new_task = tasks.emplace_back(task);

    DEBUG_PTR("Initialize context", task);
    DEBUG_PTR("New task", new_task);
    if (new_task.read_data() != 0) {
      std::cerr << "Warning: Failed to read initial data for task."
                << std::endl;
      // Optional: tasks.pop_back(); // Remove failed task if strict
    }
  }

  sleep_until = std::chrono::steady_clock::now();

  for (SystemMetrics& task : tasks) {
    DEBUG_PTR("Initialize Task", task);
    for (std::unique_ptr<IPollingTask>& polling_task : task.polling_tasks) {
      DEBUG_PTR("Polling task address", polling_task);
      polling_task->take_snapshot_1();
    }
  }
  return 0;
}
void ParsedConfig::done(std::list<SystemMetrics>& result) {
  // 2. Simply execute it.
  // The 'active_pipeline' already knows what to do and holds the necessary
  // settings/serializer.

  if (this->active_pipeline) {
    DEBUG_PTR("Active pipeline", this->active_pipeline);
    DEBUG_PTR("Active pipeline result", result);
    this->active_pipeline(result);
  }
}

// --- JSON FACTORY ---
OutputPipeline configure_json_pipeline(const MetricSettings& settings) {
  // 1. Create the Serializer with the specific settings
  // We use a shared_ptr so the lambda can capture it cheaply and keep it alive
  auto serializer = std::make_shared<JsonSerializer>(settings);

  // 2. Return the executable function
  return [serializer](const std::list<SystemMetrics>& result) {
    nlohmann::json output_json = nlohmann::json::array();

    // The serializer logic is already baked in; no 'if' checks needed here
    for (const auto& metrics : result) {
      DEBUG_PTR("configure_json_pipeline lambda Metrics", metrics);
      output_json.push_back(serializer->serialize(metrics));
    }

    std::cout << output_json.dump() << std::endl;
  };
}

// --- CONKY FACTORY ---
OutputPipeline configure_conky_pipeline(const MetricSettings& settings) {
  // 1. Capture settings by value for the lambda
  // (Or build a helper vector of print-functions like we did for JSON)
  return [settings](const std::list<SystemMetrics>& result) {
    for (const SystemMetrics& metrics : result) {
      DEBUG_PTR("configure_conky_pipeline lambda Metrics", metrics);
      print_metrics(metrics);
    }
    // for (const auto& m : result) {
    //   // This runs the loop, but using the captured settings
    //   if (settings.enable_sysinfo) {
    //     std::cout << "Node: " << m.node_name << std::endl;
    //   }
    //   if (settings.enable_cpu_temp) {
    //     std::cout << "Temp: " << m.cpu_temp_c << "C" << std::endl;
    //   }
    //   // ... etc ...
    // }
  };
}
