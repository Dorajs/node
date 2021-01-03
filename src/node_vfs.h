//
// Created by linroid on 1/3/21.
//

#ifndef NODE_VIRTUAL_FILE_SYSTEM_H
#define NODE_VIRTUAL_FILE_SYSTEM_H

#include <map>
#include <string>
#include "uv.h"

namespace node {
namespace fs {

class VirtualFileSystem {
 public:
  static const int kNone = 0;
  static const int kRead = 1;
  static const int kWrite = 2;

  VirtualFileSystem(uv_loop_t* loop);

  bool Access(const char* path, int mask, char* real_path);

  void Mount(const char* path, int mask);

  std::string Cwd();

  bool Chdir(const char* path);

 private:
  std::map<const std::string, int, std::greater<std::string>> accesses_;
  std::string cwd_;
  uv_loop_t* loop_;
};

}// namespace fs
}// namespace node

#endif  // NODE_VIRTUAL_FILE_SYSTEM_H
