//
// Created by linroid on 1/3/21.
//

#include "node_vfs.h"
#include <cstring>
#include "node_file.h"

namespace node {
namespace fs {

VirtualFileSystem::VirtualFileSystem(uv_loop_t* loop) : loop_(loop) {}

bool VirtualFileSystem::Access(const char* path, int mode, char* real_path) {
  fprintf(stdout, "access: %s, mode=%d\n, root=%s", path, mode, root_.c_str());
  if (root_.size() == 0) {
    // the chroot() doesn't get called
    return 0;
  }
  char fullp[cwd_.size() + strlen(path) + 8];
  const char* pth = path;
  while (*pth == ' ' || *pth == '\t') pth++;
  if (*pth != '/') {
    strcpy(fullp, cwd_.c_str());
    strcpy(fullp + cwd_.size(), "/");
    strcpy(fullp + cwd_.size() + 1, pth);
    pth = fullp;
  }

  //  if (*path != '/') {
  //    spath = cwd_ + '/' + spath;
  //    char buf[PATH_MAX_BYTES];
  //    spath = std::string(realpath(spath.c_str(), buf));
  //  }
  for (const auto& entry : nodes_) {
    if (spath.find(entry.first.c_str()) == 0) {
      if ((mode & entry.second.mode) == mode) {
        FSReqWrapSync req_wrap;
        //  env->PrintSyncTrace();
        int err = uv_fs_realpath(loop_, &req_wrap.req, entry.second.source.c_str(), nullptr);
        const char* link_path;
        if (err == 0) {
          strcpy(real_path, static_cast<const char*>(req_wrap.req.ptr));
        } else {
          strcpy(real_path, pth);
        }
      }
    }
  }
  return true;
}

void VirtualFileSystem::Chroot(const char* path) {
  fprintf(stdout, "chroot: %s\n", path);
  root_ = std::string(path);
}

void VirtualFileSystem::Mount(const char* source,
                              const char* target,
                              int mode) {
  fprintf(stdout, "mount: %s->%s, mode=%d\n", source, mode);
  nodes_[std::string(source)] = MountNode {
    .mode = mode;
    .source = std::string(source);
  };
}

std::string VirtualFileSystem::Cwd() {
  return cwd_;
}

bool VirtualFileSystem::Chdir(const char* path) {
  fprintf(stdout, "chdir: %s\n", path);
  char real_path[cwd_.size() + strlen(path) + 8];
  if (Access(path, kRead, real_path)) {
    cwd_ = std::string(real_path);
    return true;
  }
  return false;
}

}  // namespace fs
}  // namespace node