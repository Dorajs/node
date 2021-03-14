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

struct MountNode {
  std::string src;
  int mode;
};

class VirtualFileSystem {
 public:
  static const int kNone = 0;
  static const int kRead = 1;
  static const int kWrite = 2;

  VirtualFileSystem(uv_loop_t* loop);

  bool Access(const char* path, int mode, char* realPath);

  void Mount(const char* source, const char* target, int mode);

  void Chroot(const char* path);

  std::string Cwd();

  bool Chdir(const char* path);

 private:
  std::map<const std::string, MountNode, std::greater<std::string>> nodes_;
  std::string cwd_;
  uv_loop_t* loop_;
  std::string root_;

  std::string Resolve(const char *path);
};

}  // namespace fs
}  // namespace node

#endif  // NODE_VIRTUAL_FILE_SYSTEM_H
