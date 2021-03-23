//
// Created by linroid on 1/3/21.
//

#include <cstring>
#include <sstream>
#include <vector>
#include "node_vfs.h"
#include "node_file.h"

namespace node {

constexpr char separator = kPathSeparator;

namespace fs {

VirtualFileSystem::VirtualFileSystem() {}

bool VirtualFileSystem::Access(const char *path, int mode, char *realPath) {
  if (root_.size() == 0) {
    // the chroot() doesn't get called
    strcpy(realPath, path);
    return true;
  }
  std::string absolutePath = Resolve(path);
  for (const auto &point : points_) {
    if (absolutePath.find(point.dst) == 0) {
      if ((mode & point.mode) == mode) {
        auto subPath = absolutePath.substr(point.dst.length());
        strcpy(realPath, (point.src + subPath).c_str());
        fprintf(stdout, "access %s -> %s, mode=%d\n", path, realPath, mode);
        return true;
      } else {
        fprintf(stderr, "permission denied: %s, allowed: %d, request: %d", path, point.mode, mode);
        return false;
      }
    }
  }
  strcpy(realPath, (root_ + absolutePath).c_str());
  fprintf(stdout, "fallback: path=%s, realPath=%s, mode=%d\n", path, realPath, mode);
  return true;
}

std::string VirtualFileSystem::Path(const char* path) {
  if (root_.size() == 0) {
    return std::string(path);
  }
  std::string absolutePath = Resolve(path);

  std::sort(points_.begin(), points_.end(),
            [](MountPoint const &a, MountPoint const &b) {
              return a.src > b.src;
            });
  for (const auto &point : points_) {
    if (absolutePath.find(point.src) == 0) {
      auto subPath = absolutePath.substr(point.src.length());
      return point.dst + subPath;
    }
  }
  std::sort(points_.begin(), points_.end(),
            [](MountPoint const &a, MountPoint const &b) {
              return a.dst > b.dst;
            });
  return std::string(path);
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
  if (!root_.empty()) {
    fprintf(stderr, "chroot has already been called: %s\n", root_.c_str());
    abort();
  }
  root_ = std::string(path);
  cwd_ = std::string(path);
}

void VirtualFileSystem::Mount(const char *src, const char *dst, int mode) {
  fprintf(stdout, "mount: src=%s, dst=%s, mode=%d\n", src, dst, mode);

  MountPoint point{
      .src = std::string(src),
      .dst = std::string(dst),
      .mode = mode
  };
  points_.push_back(point);
  // TODO: replace to insert operation
  std::sort(points_.begin(), points_.end(),
            [](MountPoint const &a, MountPoint const &b) {
              return a.dst > b.dst;
            });
}

std::string VirtualFileSystem::Cwd() {
  return cwd_;
}

bool VirtualFileSystem::Chdir(const char *path) {
  char realPath[PATH_MAX_BYTES];
  if (Access(path, kRead, realPath)) {
    fprintf(stdout, "chdir(%s): realPath=%s\n", path, realPath);
    cwd_ = std::string(path);
    return true;
  }
  fprintf(stderr, "chdir failed: %s\n", path);
  return false;
}

}  // namespace fs
}  // namespace node