#include "ccp.h"
#include "comma_split.h"
#include <CLI/App.hpp>
#include <cy/chrono/stopwatch.h>
#include <exception>
#include <filesystem>
#include <iostream>
#include <spdlog/spdlog.h>
#include <string>

int main(int argc, char **argv) {
  spdlog::default_logger()->set_pattern("%v [%s:%#]");
  cy::chrono::Stopwatch stopwatch;

  std::string source_path;
  std::string destination_path;
  std::string dir_filters;
  std::string extension_filters;
  int thread_count{};
  bool copy_all_file{false};
  bool dry_run{false};

  {

    CLI::App cli_opts{"Copy lots small file with fast speed", "ccp"};
    cli_opts.add_option("--source", source_path, "path for source")->required();
    cli_opts
        .add_option("--destination", destination_path, "path for destination")
        ->required();
    cli_opts.add_option("-t,--thread", thread_count, "path for source")->default_val(0);

    cli_opts.add_flag("--all", copy_all_file,
                      "copy all file, ignore the filters.");
    cli_opts.add_flag("--dry-run", dry_run, "Dry run, not copy anything.");
    cli_opts.add_option("--dir_filters", dir_filters, "exclude dirs.");
    cli_opts.add_option("--extension_filters", extension_filters,
                        "exclude files with extensions [like '.png;.mp3'].");
    try {
      cli_opts.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
      SPDLOG_CRITICAL("failed to prase cli arg: {} ({})", e.what(),
                      e.get_name());
      return cli_opts.exit(e);
    }
  }

  try {

    Ccp_arg config;
    config.thread_count = thread_count;
    config.dry_run = dry_run;
    // prepare to remove those built-in rules or add a switch for these.
    config.dir_name_to_exclude = {".cache",  ".empty",    "bin",
                                  "build",   ".dbus",     "vcpkg",
                                  "AppData", "Downloads", "Documents"};
    config.file_extension_to_exclude = {".gz",  ".tar", ".mp4", ".dll",
                                        ".exe", ".zip", ".pdf"};
    if (!copy_all_file) {
      config.file_extension_to_exclude = concat(
          config.file_extension_to_exclude, split_comma(extension_filters));
      for (auto ex : config.file_extension_to_exclude) {
        SPDLOG_INFO("exclude extension: '{}'", ex);
      }
    }
    if (!copy_all_file) {
      config.dir_name_to_exclude =
          concat(config.dir_name_to_exclude, split_comma(dir_filters));
      for (auto ex : config.dir_name_to_exclude) {
        SPDLOG_INFO("exclude dir: '{}'", ex);
      }
    }
    if (copy_all_file) {
      config.file_extension_to_exclude.clear();
      config.dir_name_to_exclude.clear();
      SPDLOG_INFO("all files enabled, clean filters");
    }
    if (dry_run) {
      SPDLOG_INFO("dry run mode.");
    }
    ccp(source_path, destination_path, config);
  } catch (std::exception &e) {
    std::cout << e.what() << "\n";
  }
  SPDLOG_INFO("Time: {}s", stopwatch.get_elapsed_time());
}