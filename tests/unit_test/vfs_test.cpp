/**
 * @copyright Copyright The SimpleKernel Contributors
 * @brief VFS 单元测试
 */

#include "vfs.hpp"

#include <gtest/gtest.h>

#include "file_descriptor.hpp"
#include "filesystem.hpp"
#include "mount.hpp"
#include "test_environment_state.hpp"

using namespace filesystem;
using namespace vfs;

// Mock FileOps for testing
class MockFileOps : public FileOps {
 public:
  auto Read(File*, void*, size_t) -> Expected<size_t> override {
    return std::unexpected(Error(ErrorCode::kDeviceNotSupported));
  }
  auto Write(File*, const void*, size_t) -> Expected<size_t> override {
    return std::unexpected(Error(ErrorCode::kDeviceNotSupported));
  }
  auto Seek(File*, int64_t, SeekWhence) -> Expected<uint64_t> override {
    return std::unexpected(Error(ErrorCode::kDeviceNotSupported));
  }
  auto Close(File*) -> Expected<void> override { return {}; }
  auto ReadDir(File*, DirEntry*, size_t) -> Expected<size_t> override {
    return std::unexpected(Error(ErrorCode::kDeviceNotSupported));
  }
};

// Mock file system for testing
class MockFs : public FileSystem {
 public:
  mutable bool mount_called = false;
  mutable bool unmount_called = false;
  mutable bool sync_called = false;
  mutable BlockDevice* last_device = nullptr;
  MockFileOps mock_file_ops_;

  Inode root_inode;

  MockFs() {
    root_inode.type = FileType::kDirectory;
    root_inode.ino = 1;
    root_inode.ops = nullptr;  // MockFs doesn't provide InodeOps
  }

  [[nodiscard]] auto GetName() const -> const char* override {
    return "mockfs";
  }

  auto Mount(BlockDevice* device) -> Expected<Inode*> override {
    mount_called = true;
    last_device = device;
    return &root_inode;
  }

  auto Unmount() -> Expected<void> override {
    unmount_called = true;
    return {};
  }

  auto Sync() -> Expected<void> override {
    sync_called = true;
    return {};
  }

  auto AllocateInode() -> Expected<Inode*> override {
    return std::unexpected(Error(ErrorCode::kOutOfMemory));
  }

  auto FreeInode(Inode* /*inode*/) -> Expected<void> override { return {}; }

  auto GetFileOps() -> FileOps* override { return &mock_file_ops_; }
};

// VFS 基础测试
class VfsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    env_state_.InitializeCores(1);
    env_state_.SetCurrentThreadEnvironment();
    env_state_.BindThreadToCore(std::this_thread::get_id(), 0);
    auto result = vfs::Init();
    EXPECT_TRUE(result.has_value());
  }

  void TearDown() override { env_state_.ClearCurrentThreadEnvironment(); }

  test_env::TestEnvironmentState env_state_;
};

class BaseEnvTest : public ::testing::Test {
 protected:
  void SetUp() override {
    env_state_.InitializeCores(1);
    env_state_.SetCurrentThreadEnvironment();
    env_state_.BindThreadToCore(std::this_thread::get_id(), 0);
  }
  void TearDown() override { env_state_.ClearCurrentThreadEnvironment(); }
  test_env::TestEnvironmentState env_state_;
};

// 测试挂载表
class MountTableTest : public BaseEnvTest {};
TEST_F(MountTableTest, MountAndUnmount) {
  MountTable mount_table;
  MockFs mock_fs;

  // 测试挂载
  auto mount_result = mount_table.Mount("/", &mock_fs, nullptr);
  EXPECT_TRUE(mount_result.has_value());
  EXPECT_TRUE(mock_fs.mount_called);

  // 测试重复挂载
  mount_result = mount_table.Mount("/", &mock_fs, nullptr);
  EXPECT_FALSE(mount_result.has_value());

  // 测试卸载
  auto unmount_result = mount_table.Unmount("/");
  (void)unmount_result;
  EXPECT_TRUE(unmount_result.has_value());
  EXPECT_TRUE(mock_fs.unmount_called);

  // 测试卸载未挂载的路径
  unmount_result = mount_table.Unmount("/mnt");
  EXPECT_FALSE(unmount_result.has_value());
}

TEST_F(MountTableTest, LookupMountPoint) {
  MountTable mount_table;
  MockFs mock_fs;

  auto mount_result = mount_table.Mount("/", &mock_fs, nullptr);
  EXPECT_TRUE(mount_result.has_value());

  // 查找根挂载点
  auto* mp = mount_table.Lookup("/file.txt");
  EXPECT_NE(mp, nullptr);

  // 查找不存在挂载的路径
  mp = mount_table.Lookup("/mnt/nonexistent/file");
  EXPECT_NE(mp, nullptr);  // 应该返回根挂载点
  (void)mount_table.Unmount("/");
}

// 测试文件描述符表
class FdTableTest : public BaseEnvTest {
 protected:
  FileDescriptorTable* fd_table_;

  void SetUp() override {
    BaseEnvTest::SetUp();
    fd_table_ = new FileDescriptorTable();
  }

  void TearDown() override {
    delete fd_table_;
    BaseEnvTest::TearDown();
  }
};

TEST_F(FdTableTest, AllocAndFree) {
  // 创建模拟文件
  File mock_file;

  // 分配 fd
  auto alloc_result = fd_table_->Alloc(&mock_file);
  EXPECT_TRUE(alloc_result.has_value());
  int fd = alloc_result.value();
  EXPECT_GE(fd, 3);  // 0/1/2 预留给标准流

  // 获取文件
  File* file = fd_table_->Get(fd);
  EXPECT_EQ(file, &mock_file);

  // 释放 fd
  auto free_result = fd_table_->Free(fd);
  EXPECT_TRUE(free_result.has_value());

  // 再次获取应该返回 nullptr
  file = fd_table_->Get(fd);
  EXPECT_EQ(file, nullptr);
}

TEST_F(FdTableTest, InvalidFd) {
  // 无效 fd
  File* file = fd_table_->Get(-1);
  EXPECT_EQ(file, nullptr);

  file = fd_table_->Get(999);
  EXPECT_EQ(file, nullptr);

  // 释放无效 fd
  auto free_result = fd_table_->Free(-1);
  EXPECT_FALSE(free_result.has_value());
}

TEST_F(FdTableTest, DupFd) {
  File mock_file;

  // 分配 fd
  auto alloc_result = fd_table_->Alloc(&mock_file);
  EXPECT_TRUE(alloc_result.has_value());
  int fd1 = alloc_result.value();

  // 复制 fd
  auto dup_result = fd_table_->Dup(fd1);
  EXPECT_TRUE(dup_result.has_value());
  int fd2 = dup_result.value();

  // fd1 和 fd2 应该指向同一个文件
  EXPECT_EQ(fd_table_->Get(fd1), fd_table_->Get(fd2));

  // 清理
  (void)fd_table_->Free(fd1);
  (void)fd_table_->Free(fd2);
}

TEST_F(FdTableTest, SetupStandardFiles) {
  File stdin_file;
  File stdout_file;
  File stderr_file;

  auto setup_result =
      fd_table_->SetupStandardFiles(&stdin_file, &stdout_file, &stderr_file);
  EXPECT_TRUE(setup_result.has_value());

  // 检查标准文件描述符
  EXPECT_EQ(fd_table_->Get(0), &stdin_file);
  EXPECT_EQ(fd_table_->Get(1), &stdout_file);
  EXPECT_EQ(fd_table_->Get(2), &stderr_file);
}

// VFS 路径解析测试
TEST_F(VfsTest, LookupRoot) {
  // 挂载 mock 文件系统作为根
  MockFs mock_fs;
  auto& mount_table = GetMountTable();

  auto mount_result = mount_table.Mount("/", &mock_fs, nullptr);
  EXPECT_TRUE(mount_result.has_value());

  // 查找根目录
  auto lookup_result = vfs::Lookup("/");
  EXPECT_TRUE(lookup_result.has_value());
  EXPECT_NE(lookup_result.value(), nullptr);
}

TEST_F(VfsTest, LookupInvalidPaths) {
  // 空路径
  auto result = vfs::Lookup(nullptr);
  EXPECT_FALSE(result.has_value());

  // 相对路径
  result = vfs::Lookup("relative/path");
  EXPECT_FALSE(result.has_value());
}

// VFS 初始化测试
class VfsInitTest : public BaseEnvTest {};
TEST_F(VfsInitTest, DoubleInit) {
  // 第一次初始化
  auto result = vfs::Init();
  EXPECT_TRUE(result.has_value());

  // 重复初始化应该成功（幂等）
  result = vfs::Init();
  EXPECT_TRUE(result.has_value());
}

// File 结构测试
TEST(FileStructTest, FileOperations) {
  File file;
  EXPECT_EQ(file.offset, 0);
  EXPECT_EQ(file.flags, 0);
  EXPECT_EQ(file.ops, nullptr);
  EXPECT_EQ(file.inode, nullptr);
  EXPECT_EQ(file.dentry, nullptr);
}

// Inode 结构测试
TEST(InodeStructTest, InodeDefaults) {
  Inode inode;
  EXPECT_EQ(inode.ino, 0);
  EXPECT_EQ(inode.type, FileType::kUnknown);
  EXPECT_EQ(inode.size, 0);
  EXPECT_EQ(inode.permissions, 0644);
  EXPECT_EQ(inode.link_count, 1);
  EXPECT_EQ(inode.fs_private, nullptr);
  EXPECT_EQ(inode.fs, nullptr);
  EXPECT_EQ(inode.ops, nullptr);
}

// Dentry 结构测试
TEST(DentryStructTest, DentryDefaults) {
  Dentry dentry;
  EXPECT_EQ(dentry.name[0], '\0');
  EXPECT_EQ(dentry.inode, nullptr);
  EXPECT_EQ(dentry.parent, nullptr);
  EXPECT_EQ(dentry.children, nullptr);
  EXPECT_EQ(dentry.next_sibling, nullptr);
  EXPECT_EQ(dentry.fs_private, nullptr);
}

// OpenFlags 测试
TEST(OpenFlagsTest, FlagValues) {
  EXPECT_EQ(static_cast<uint32_t>(OpenFlags::kOReadOnly), 0x0000u);
  EXPECT_EQ(static_cast<uint32_t>(OpenFlags::kOWriteOnly), 0x0001u);
  EXPECT_EQ(static_cast<uint32_t>(OpenFlags::kOReadWrite), 0x0002u);
  EXPECT_EQ(static_cast<uint32_t>(OpenFlags::kOCreate), 0x0040u);
  EXPECT_EQ(static_cast<uint32_t>(OpenFlags::kOTruncate), 0x0200u);
  EXPECT_EQ(static_cast<uint32_t>(OpenFlags::kOAppend), 0x0400u);
  EXPECT_EQ(static_cast<uint32_t>(OpenFlags::kODirectory), 0x010000u);
}

// SeekWhence 测试
TEST(SeekWhenceTest, EnumValues) {
  EXPECT_EQ(static_cast<int>(SeekWhence::kSet), 0);
  EXPECT_EQ(static_cast<int>(SeekWhence::kCur), 1);
  EXPECT_EQ(static_cast<int>(SeekWhence::kEnd), 2);
}
