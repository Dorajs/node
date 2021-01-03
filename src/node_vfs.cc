//
// Created by linroid on 1/3/21.
//

#include "node_vfs.h"
#include "node_file.h"
#include <cstring>

namespace node {
namespace fs {

VirtualFileSystem::VirtualFileSystem(uv_loop_t* loop) : loop_(loop) {

}
bool VirtualFileSystem::Access(const char *path, int mask, char * real_path) {
//  fprintf(stdout, "access: %s, mask=%d\n", path, mask);
  char fullp[cwd_.size() + strlen(path) + 8];
  const char *pth = path;
  while (*pth == ' ' || *pth == '\t') pth++;
  if (*pth != '/') {
    strcpy(fullp, cwd_.c_str());
    strcpy(fullp + cwd_.size(), "/");
    strcpy(fullp + cwd_.size() + 1, pth);
    pth = fullp;
  }
  FSReqWrapSync req_wrap;
  //  env->PrintSyncTrace();
  int err = uv_fs_realpath(loop_, &req_wrap.req, pth, nullptr);
  const char* link_path;
  if (err != 0) {
    link_path = pth;
  } else {
    link_path = static_cast<const char*>(req_wrap.req.ptr);
  }
  strcpy(real_path, link_path);

  std::string spath(link_path);

//  if (*path != '/') {
//    spath = cwd_ + '/' + spath;
//    char buf[PATH_MAX_BYTES];
//    spath = std::string(realpath(spath.c_str(), buf));
//  }
  for (const auto &kv: accesses_) {
    if (spath.find(kv.first.c_str()) == 0) {
      return (mask & kv.second) == mask;
    }
  }
  return true;
}

void VirtualFileSystem::Mount(const char *path, int mask) {
//  fprintf(stdout, "mount: %s, mask=%d\n", path, mask);
  accesses_[std::string(path)] = mask;
}

std::string VirtualFileSystem::Cwd() {
  return cwd_;
}

bool VirtualFileSystem::Chdir(const char *path) {
//  fprintf(stdout, "chdir: %s\n", path);
  char real_path[cwd_.size() + strlen(path) + 8];
  if (Access(path, kRead, real_path)) {
    cwd_ = std::string(real_path);
    return true;
  }
  return false;
}

} // namespace node
} // namespace fs