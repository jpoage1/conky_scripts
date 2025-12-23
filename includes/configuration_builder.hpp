// configuration_builder.hpp
#ifndef CONFIGURATION_BUILDER_HPP
#define CONFIGURATION_BUILDER_HPP
struct ProgramOptions;
struct Command;
struct MetricsContext;
struct CommandRequest;

class ParsedConfig;

/**
 * Path 2: CLI-to-Config Translation
 * Encapsulates the logic of converting CLI options into a runnable
 * configuration.
 */
ParsedConfig build_config_from_options(const ProgramOptions& options);
/**
 * Helper to isolate specific command mapping.
 * This is now individually testable.
 */
MetricsContext create_context_from_command(const CommandRequest& cmd);

#endif
