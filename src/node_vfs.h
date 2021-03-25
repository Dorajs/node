//
// Created by linroid on 1/3/21.
//

#ifndef NODE_VIRTUAL_FILE_SYSTEM_H
#define NODE_VIRTUAL_FILE_SYSTEM_H

#include <vector>
#include <string>

namespace node {
namespace fs {

struct MountPoint {
  std::string src;
  std::string dst;
  int mode;
};

class VirtualFileSystem {
public:
  static const int kNone = 0;
  static const int kRead = 1;
  static const int kWrite = 2;

  VirtualFileSystem();

  /**
   * Access the virtual path with special permission mode
   * @param path the virtual path
   * @param mode permission
   * @return the real path on host, empty if no permission to access the path
   */
  std::string Access(const char *path, int mode);

  /**
   * Convert the real path to virtual path
   * @param path the virtual path
   * @return
   */
  std::string Path(const char *path);

  void Mount(const char *src, const char *dst, int mode);

  void Chroot(const char *path);

  bool Chdir(const char *path);

  std::string Cwd();

private:
  // key is the dst path
  std::vector <MountPoint> points_;
  std::string cwd_;
  std::string root_;

  std::string Resolve(const char *path);
};

}  // namespace fs
}  // namespace node

#endif  // NODE_VIRTUAL_FILE_SYSTEM_H
