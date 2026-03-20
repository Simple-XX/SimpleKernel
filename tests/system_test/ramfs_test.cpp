/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cstddef>
#include <cstdint>

#include "kstd_cstring"
#include "system_test.h"
#include "vfs.hpp"

auto ramfs_test() -> bool {
  klog::Info("ramfs_test: start");

  // FileSystemInit() has already been called in main.cpp.
  // ramfs is mounted at "/" — use VFS directly.

  // T1: Create file, write, read back
  {
    auto file_result = vfs::Open(
        "/hello.txt", vfs::OpenFlags::kOCreate | vfs::OpenFlags::kOReadWrite);
    EXPECT_TRUE(file_result.has_value(), "ramfs_test: open /hello.txt failed");
    vfs::File* file = file_result.value();

    const char kMsg[] = "Hello, ramfs!";
    auto write_result = vfs::Write(file, kMsg, sizeof(kMsg) - 1);
    EXPECT_TRUE(write_result.has_value(), "ramfs_test: write failed");
    EXPECT_EQ(write_result.value(), sizeof(kMsg) - 1,
              "ramfs_test: write byte count mismatch");
    klog::Info("ramfs_test: wrote {} bytes", write_result.value());

    // Seek back to start
    auto seek_result = vfs::Seek(file, 0, vfs::SeekWhence::kSet);
    EXPECT_TRUE(seek_result.has_value(), "ramfs_test: seek to start failed");
    EXPECT_EQ(seek_result.value(), static_cast<uint64_t>(0),
              "ramfs_test: seek position mismatch");

    // Read back
    char buf[64] = {};
    auto read_result = vfs::Read(file, buf, sizeof(buf) - 1);
    EXPECT_TRUE(read_result.has_value(), "ramfs_test: read failed");
    EXPECT_EQ(read_result.value(), sizeof(kMsg) - 1,
              "ramfs_test: read byte count mismatch");
    EXPECT_EQ(memcmp(buf, kMsg, sizeof(kMsg) - 1), 0,
              "ramfs_test: read content mismatch");
    klog::Info("ramfs_test: read back: {}", buf);

    (void)vfs::Close(file);
  }

  // T3: Seek to middle, partial read
  {
    auto file_result = vfs::Open("/hello.txt", vfs::OpenFlags::kOReadOnly);
    EXPECT_TRUE(file_result.has_value(),
                "ramfs_test: re-open for seek test failed");
    vfs::File* file = file_result.value();

    // Seek to offset 7
    auto seek_result = vfs::Seek(file, 7, vfs::SeekWhence::kSet);
    EXPECT_TRUE(seek_result.has_value(), "ramfs_test: seek to offset 7 failed");
    EXPECT_EQ(seek_result.value(), static_cast<uint64_t>(7),
              "ramfs_test: seek offset 7 mismatch");

    char buf[32] = {};
    auto read_result = vfs::Read(file, buf, 5);
    EXPECT_TRUE(read_result.has_value(), "ramfs_test: partial read failed");
    // "Hello, ramfs!" -> offset 7 = "ramfs"
    EXPECT_EQ(read_result.value(), static_cast<size_t>(5),
              "ramfs_test: partial read count mismatch");
    EXPECT_EQ(memcmp(buf, "ramfs", 5), 0,
              "ramfs_test: partial read content mismatch");
    klog::Info("ramfs_test: partial read from offset 7: {}", buf);

    (void)vfs::Close(file);
  }

  // T4: MkDir + ReadDir
  {
    auto mkdir_result = vfs::MkDir("/testdir");
    EXPECT_TRUE(mkdir_result.has_value(), "ramfs_test: mkdir /testdir failed");
    klog::Info("ramfs_test: mkdir /testdir ok");

    // Create a file inside
    auto inner =
        vfs::Open("/testdir/inner.txt",
                  vfs::OpenFlags::kOCreate | vfs::OpenFlags::kOWriteOnly);
    EXPECT_TRUE(inner.has_value(),
                "ramfs_test: open /testdir/inner.txt failed");
    (void)vfs::Close(inner.value());

    // ReadDir on /testdir
    auto dir_file_result = vfs::Open(
        "/testdir", vfs::OpenFlags::kOReadOnly | vfs::OpenFlags::kODirectory);
    EXPECT_TRUE(dir_file_result.has_value(),
                "ramfs_test: open /testdir as dir failed");
    vfs::File* dir_file = dir_file_result.value();

    vfs::DirEntry entries[8] = {};
    auto readdir_result = vfs::ReadDir(dir_file, entries, 8);
    EXPECT_TRUE(readdir_result.has_value(), "ramfs_test: readdir failed");
    // Expect at least "." + ".." + "inner.txt" = 3 entries
    EXPECT_GT(readdir_result.value(), static_cast<size_t>(2),
              "ramfs_test: readdir should return > 2 entries");
    klog::Info("ramfs_test: readdir returned {} entries",
               readdir_result.value());

    (void)vfs::Close(dir_file);
  }

  // T5: Unlink a file, confirm it can't be re-opened without kOCreate
  {
    auto unlink_result = vfs::Unlink("/hello.txt");
    EXPECT_TRUE(unlink_result.has_value(),
                "ramfs_test: unlink /hello.txt failed");
    klog::Info("ramfs_test: unlink /hello.txt ok");

    auto reopen = vfs::Open("/hello.txt", vfs::OpenFlags::kOReadOnly);
    EXPECT_FALSE(reopen.has_value(),
                 "ramfs_test: /hello.txt should be gone after unlink");
    klog::Info("ramfs_test: confirmed /hello.txt no longer exists");
  }

  // T6: RmDir
  {
    // Remove inner file first
    auto unlink_result = vfs::Unlink("/testdir/inner.txt");
    EXPECT_TRUE(unlink_result.has_value(),
                "ramfs_test: unlink /testdir/inner.txt failed");

    auto rmdir_result = vfs::RmDir("/testdir");
    EXPECT_TRUE(rmdir_result.has_value(), "ramfs_test: rmdir /testdir failed");
    klog::Info("ramfs_test: rmdir /testdir ok");
  }

  // T7: Two independent files do not share data
  {
    auto f1 = vfs::Open("/fileA.txt",
                        vfs::OpenFlags::kOCreate | vfs::OpenFlags::kOReadWrite);
    auto f2 = vfs::Open("/fileB.txt",
                        vfs::OpenFlags::kOCreate | vfs::OpenFlags::kOReadWrite);
    EXPECT_TRUE(f1.has_value(), "ramfs_test: open fileA failed");
    EXPECT_TRUE(f2.has_value(), "ramfs_test: open fileB failed");

    const char kDataA[] = "AAAA";
    const char kDataB[] = "BBBB";
    (void)vfs::Write(f1.value(), kDataA, 4);
    (void)vfs::Write(f2.value(), kDataB, 4);

    (void)vfs::Seek(f1.value(), 0, vfs::SeekWhence::kSet);
    (void)vfs::Seek(f2.value(), 0, vfs::SeekWhence::kSet);

    char buf1[8] = {};
    char buf2[8] = {};
    (void)vfs::Read(f1.value(), buf1, 4);
    (void)vfs::Read(f2.value(), buf2, 4);

    EXPECT_EQ(memcmp(buf1, kDataA, 4), 0,
              "ramfs_test: fileA data corrupted by fileB");
    EXPECT_EQ(memcmp(buf2, kDataB, 4), 0,
              "ramfs_test: fileB data corrupted by fileA");
    klog::Info("ramfs_test: two files are independent");

    (void)vfs::Close(f1.value());
    (void)vfs::Close(f2.value());
    (void)vfs::Unlink("/fileA.txt");
    (void)vfs::Unlink("/fileB.txt");
  }

  klog::Info("ramfs_test: all tests passed");
  return true;
}
