//
// Created by linroid on 1/3/21.
//

#include "node_vfs.h"
#include <cstring>
#include <sstream>
#include <vector>
#include "node_file.h"

constexpr char separator = '/';

namespace node {
namespace fs {

VirtualFileSystem::VirtualFileSystem(uv_loop_t *loop) : loop_(loop) {}

bool VirtualFileSystem::Access(const char *path, int mode, char *realPath) {
  fprintf(stdout, "access: %s, mode=%d, root=%s", path, mode, root_.c_str());
  if (root_.size() == 0) {
    // the chroot() doesn't get called
    strcpy(realPath, path);
    return true;
  }

  std::string absolutePath = Resolve(path);
  for (const auto &entry : nodes_) {
    if (absolutePath.find(entry.first) == 0) {
      if ((mode & entry.second.mode) == mode) {
        std::string src = entry.second.src + absolutePath.substr(entry.first.length());
        fprintf(stdout, "path=%s, src=%s", path, src.c_str());
        strcpy(realPath, src.c_str());
        // env->PrintSyncTrace();
        // FSReqWrapSync req_wrap;
        // int err = uv_fs_realpath(loop_, &req_wrap.req, src.c_str(), nullptr);
        // if (err == 0) {
        //   fprintf(stdout, "real path is %s", static_cast<const char *>(req_wrap.req.ptr));
        //   strcpy(realPath, static_cast<const char *>(req_wrap.req.ptr));
        // } else {
        //   strcpy(realPath, absolutePath.c_str());
        // }
        return true;
      } else {
        fprintf(stderr, "permission denied: %s, allowed: %d, request: %d", path, entry.second.mode, mode);
        return false;
      }
    }
  }
  return false;
}

std::string VirtualFileSystem::Resolve(const char *path) {
  // join the path to cwd
  std::stringstream stream;
  if (*path != separator && !cwd_.empty()) {
    stream << cwd_;
    if (cwd_[cwd_.length() - 1] != separator) {
      stream << separator;
    }
  }
  stream << path;
  std::cout << stream.str() << std::endl;

  // split the path by separator
  std::vector <std::string> segments;
  for (;;) {
    std::string segment;
    if (!std::getline(stream, segment, separator)) {
      break;
    }
    segments.push_back(segment);
  }

  // remove relative segments
  std::vector <std::string> pruned;
  for (auto &segment : segments) {
    if (segment.empty() || segment == ".") {
      continue;
    }
    if (segment == "..") {
      pruned.pop_back();
      continue;
    }
    pruned.push_back(segment);
  }

  // get the sanitized path
  std::stringstream result;
  for (auto &segment : pruned) {
    result << separator << segment;
  }
  if (strlen(path) > 0 && path[strlen(path) - 1] == separator) {
    result << separator;
  }
  return result.str();
}

void VirtualFileSystem::Chroot(const char *path) {
  fprintf(stdout, "chroot: %s\n", path);
  root_ = std::string(path);
  cwd_ = std::string(path);
  nodes_[std::string("/")] = MountNode{.src = root_, .mode = kRead | kWrite};
}

void VirtualFileSystem::Mount(const char *src,
                              const char *dst,
                              int mode) {
  fprintf(stdout, "mount: %s->%s, mode=%d\n", src, dst, mode);

  nodes_[std::string(dst)] = MountNode{.src = std::string(src), .mode = mode};
}

std::string VirtualFileSystem::Cwd() {
  return cwd_;
}

bool VirtualFileSystem::Chdir(const char *path) {
  fprintf(stdout, "chdir: %s\n", path);
  char realPath[cwd_.size() + strlen(path) + 8];
  if (Access(path, kRead, realPath)) {
    cwd_ = std::string(realPath);
    return true;
  }
  return false;
}

}  // namespace fs
}  // namespace node