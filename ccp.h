#pragma once

// #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_OFF
#include <algorithm>
#include <atomic>
#include <chrono>
#include <exception>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include <string>
#include <system_error>
#include <thread>
#include <vcruntime.h>
#include <vector>
#include "concurrentqueue/blockingconcurrentqueue.h"
struct Ccp_arg {
  std::vector<std::string> dir_to_exclude;
  std::vector<std::string> file_extension_to_exclude;
};
inline bool ccp(std::filesystem::path in, std::filesystem::path out,
                Ccp_arg args) {
  SPDLOG_INFO("copy data from {} to {}", in.string(), out.string());
  std::atomic<size_t> total_size {};
  moodycamel::BlockingConcurrentQueue<std::function<void()>> queue;
  std::atomic<bool> loading = true;
  uint64_t ct{};
  const int max_queue_limit = 40960;

  std::vector<std::unique_ptr<std::thread>> threads;

for(int i=0;i < 16;i++){
    threads.push_back(std::make_unique<std::thread>([&]{
        while(loading){
            std::function<void()> f;
            auto ok = queue.wait_dequeue_timed(f,std::chrono::milliseconds{10});
            if(ok){
                f();
            }
        }
    }));
}


  std::filesystem::recursive_directory_iterator begin{in}, end{};
  while (begin != end) {
    bool stop_adding = false;
    const auto current = begin->path();
    bool is_dir = std::filesystem::is_directory(*begin);
    auto current_name = begin->path().filename();
    // SPDLOG_INFO("tasking {} {}", current.string(), is_dir ? "[dir]" : "");
    if (is_dir) {

      bool skip_it =
          std::any_of(args.dir_to_exclude.begin(), args.dir_to_exclude.end(),
                      [current_name](auto &n) { return n == current_name; });
      if (skip_it) {
        begin++;
        if (begin != end) {
          if (begin->path().parent_path().filename() == current_name) {
            // SPDLOG_INFO("skip dir: {}", current.string());
            begin.pop();
          }
        }
        stop_adding = true;
      }
    } else {
    //   auto relative_in_real_path = std::filesystem::relative(current, in);
      auto relative_in_path = current.lexically_relative(in);
      auto output_path = out / relative_in_path;
      // SPDLOG_DEBUG("tasking copy {} to {} [{}]", current.string(),
                  // output_path.string(), relative_in_path.string());
      auto current_ext = current_name.extension();
      // SPDLOG_DEBUG("Ext: '{}'",current_ext.string());
      bool skip_it =
          std::any_of(args.file_extension_to_exclude.begin(),
                      args.file_extension_to_exclude.end(),
                      [current_ext](const auto &n) { return n == current_ext; });
      if (!skip_it) {
        if (output_path.has_parent_path() &&
            !std::filesystem::exists(output_path.parent_path())) {
          std::error_code ec;
          try{
            std::filesystem::create_directories(output_path.parent_path());
          }catch(std::exception & e){
            // SPDLOG_ERROR("create dir failed: {}",e.what());
            loading = false;
            throw;
          }

        }
        auto copy_call = [=,&total_size]{
            try{

        if(std::filesystem::is_regular_file(current)){
          total_size += std::filesystem::file_size(current);
            std::filesystem::copy_file(current, output_path);
        }else if(std::filesystem::is_symlink(current)){
            std::filesystem::copy_symlink(current,output_path);
        }
            }catch(std::exception&e){
                SPDLOG_ERROR("failed copy {}",e.what());
            }
        };
        while(queue.size_approx() > max_queue_limit){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        queue.enqueue(copy_call);

        ct++;
        // auto ft = std::async(std::launch::async | std::launch::deferred, copy_call); fail.
        // futures.push_back(std::move(ft));
      }else{
        // SPDLOG_DEBUG("Skip file {}",current.string());
      }
    }
    if (!stop_adding) {
      begin++;
    }
  }
   while(queue.size_approx() != 0){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
  loading = false;
  for(auto &t: threads){
    t->join();
  }
  SPDLOG_INFO("TOTAL: {}",total_size.load());
  return true;
}
