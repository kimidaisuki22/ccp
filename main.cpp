#include "ccp.h"
#include "comma_split.h"
#include <exception>
#include <filesystem>
#include <iostream>
#include <spdlog/spdlog.h>
int main(int argc, char **argv) {
  spdlog::default_logger()->set_pattern("%v [%s:%#]");
  try {
    std::string in = argv[1];
    std::string out = argv[2];
    Ccp_arg config;
    config.dir_to_exclude = {".git", ".cache", ".empty", "bin","build",".dbus", "vcpkg","AppData","Downloads","Documents"};
    config.file_extension_to_exclude = {".gz", ".tar",".mp4",".dll",".exe",".zip",".pdf"};
    if (argc >= 4) {
      config.file_extension_to_exclude = concat(config.file_extension_to_exclude, split_comma(argv[3]));
      for(auto ex: config.file_extension_to_exclude){
        SPDLOG_INFO("exclude: '{}'",ex);
      }
    //   exit(1);
    }
    if (argc >= 5) {
      config.dir_to_exclude = concat(config.dir_to_exclude, split_comma(argv[4]));
      for(auto ex: config.dir_to_exclude){
        SPDLOG_INFO("exclude dir: '{}'",ex);
      }
    //   exit(1);
    }
    ccp(in, out, config);
  } catch (std::exception &e) {
    std::cout << e.what() << "\n";
  }
}