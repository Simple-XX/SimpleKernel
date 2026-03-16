/**
 * @copyright Copyright The SimpleKernel Contributors
 * @brief ramfs 单元测试
 */

#include "ramfs.hpp"

#include <gtest/gtest.h>

#include <vector>

#include "test_environment_state.hpp"

using namespace vfs;
using namespace ramfs;

class RamFsTest : public ::testing::Test {
 protected:
  RamFs ramfs_;
  test_env::TestEnvironmentState env_state_;

  void SetUp() override {
    env_state_.InitializeCores(1);
    env_state_.SetCurrentThreadEnvironment();
    env_state_.BindThreadToCore(std::this_thread::get_id(), 0);
    auto result = ramfs_.Mount(nullptr);
    EXPECT_TRUE(result.has_value());
  }

  void TearDown() override {
    (void)ramfs_.Unmount();
    env_state_.ClearCurrentThreadEnvironment();
  }
};

// 测试基本挂载/卸载
TEST_F(RamFsTest, MountUnmount) {
  // SetUp 中已经挂载，这里测试重复挂载
  auto result = ramfs_.Mount(nullptr);
  EXPECT_FALSE(result.has_value());  // 应该失败，因为已经挂载
}

TEST_F(RamFsTest, GetName) { EXPECT_STREQ(ramfs_.GetName(), "ramfs"); }

TEST_F(RamFsTest, GetRootInode) {
  Inode* root = ramfs_.GetRootInode();
  EXPECT_NE(root, nullptr);
  EXPECT_EQ(root->type, FileType::kDirectory);
}

// 测试 inode 分配
TEST_F(RamFsTest, AllocateInode) {
  // 分配多个 inode
  std::vector<Inode*> inodes;
  for (int i = 0; i < 10; ++i) {
    auto result = ramfs_.AllocateInode();
    EXPECT_TRUE(result.has_value());
    EXPECT_NE(result.value(), nullptr);
    inodes.push_back(result.value());
  }

  // 释放所有 inode
  for (auto* inode : inodes) {
    auto free_result = ramfs_.FreeInode(inode);
    EXPECT_TRUE(free_result.has_value());
  }
}

// 测试文件创建和查找
TEST_F(RamFsTest, CreateAndLookupFile) {
  Inode* root = ramfs_.GetRootInode();
  ASSERT_NE(root, nullptr);
  ASSERT_NE(root->ops, nullptr);

  // 创建文件
  auto create_result =
      root->ops->Create(root, "testfile.txt", FileType::kRegular);
  EXPECT_TRUE(create_result.has_value());

  Inode* file_inode = create_result.value();
  EXPECT_NE(file_inode, nullptr);
  EXPECT_EQ(file_inode->type, FileType::kRegular);

  // 查找文件
  auto lookup_result = root->ops->Lookup(root, "testfile.txt");
  EXPECT_TRUE(lookup_result.has_value());
  EXPECT_EQ(lookup_result.value(), file_inode);

  // 查找不存在的文件
  lookup_result = root->ops->Lookup(root, "nonexistent.txt");
  EXPECT_FALSE(lookup_result.has_value());
}

// 测试目录创建
TEST_F(RamFsTest, CreateDirectory) {
  Inode* root = ramfs_.GetRootInode();
  ASSERT_NE(root, nullptr);
  ASSERT_NE(root->ops, nullptr);

  // 创建目录
  auto mkdir_result = root->ops->Mkdir(root, "testdir");
  EXPECT_TRUE(mkdir_result.has_value());

  Inode* dir_inode = mkdir_result.value();
  EXPECT_NE(dir_inode, nullptr);
  EXPECT_EQ(dir_inode->type, FileType::kDirectory);
}

// 测试文件删除
TEST_F(RamFsTest, UnlinkFile) {
  Inode* root = ramfs_.GetRootInode();
  ASSERT_NE(root, nullptr);

  // 先创建文件
  auto create_result =
      root->ops->Create(root, "todelete.txt", FileType::kRegular);
  ASSERT_TRUE(create_result.has_value());

  // 删除文件
  auto unlink_result = root->ops->Unlink(root, "todelete.txt");
  EXPECT_TRUE(unlink_result.has_value());

  // 确认文件已删除
  auto lookup_result = root->ops->Lookup(root, "todelete.txt");
  EXPECT_FALSE(lookup_result.has_value());
}

// 测试目录删除
TEST_F(RamFsTest, Rmdir) {
  Inode* root = ramfs_.GetRootInode();
  ASSERT_NE(root, nullptr);

  // 创建目录
  auto mkdir_result = root->ops->Mkdir(root, "dir_to_remove");
  ASSERT_TRUE(mkdir_result.has_value());

  // 删除目录
  auto rmdir_result = root->ops->Rmdir(root, "dir_to_remove");
  EXPECT_TRUE(rmdir_result.has_value());

  // 确认目录已删除
  auto lookup_result = root->ops->Lookup(root, "dir_to_remove");
  EXPECT_FALSE(lookup_result.has_value());
}

// 测试文件读写
TEST_F(RamFsTest, FileReadWrite) {
  Inode* root = ramfs_.GetRootInode();
  ASSERT_NE(root, nullptr);

  // 创建文件
  auto create_result =
      root->ops->Create(root, "rwtest.txt", FileType::kRegular);
  ASSERT_TRUE(create_result.has_value());

  Inode* file_inode = create_result.value();

  // 创建 File 对象
  File file;
  file.inode = file_inode;
  file.offset = 0;
  file.ops = ramfs_.GetFileOps();

  // 写入数据
  const char* write_data = "Hello, RamFS!";
  size_t write_len = strlen(write_data);

  auto write_result = file.ops->Write(&file, write_data, write_len);
  EXPECT_TRUE(write_result.has_value());
  EXPECT_EQ(write_result.value(), write_len);

  // 重置偏移量并读取
  file.offset = 0;
  char read_buffer[64] = {0};

  auto read_result = file.ops->Read(&file, read_buffer, sizeof(read_buffer));
  EXPECT_TRUE(read_result.has_value());
  EXPECT_EQ(read_result.value(), write_len);
  EXPECT_STREQ(read_buffer, write_data);
}

// 测试文件 seek
TEST_F(RamFsTest, FileSeek) {
  Inode* root = ramfs_.GetRootInode();
  ASSERT_NE(root, nullptr);

  // 创建文件并写入数据
  auto create_result =
      root->ops->Create(root, "seektest.txt", FileType::kRegular);
  ASSERT_TRUE(create_result.has_value());

  File file;
  file.inode = create_result.value();
  file.offset = 0;
  file.ops = ramfs_.GetFileOps();

  const char* data = "ABCDEFGHIJ";
  (void)file.ops->Write(&file, data, strlen(data));

  // SEEK_SET
  auto seek_result = file.ops->Seek(&file, 5, SeekWhence::kSet);
  EXPECT_TRUE(seek_result.has_value());
  EXPECT_EQ(seek_result.value(), 5u);

  // SEEK_CUR
  seek_result = file.ops->Seek(&file, 2, SeekWhence::kCur);
  EXPECT_TRUE(seek_result.has_value());
  EXPECT_EQ(seek_result.value(), 7u);

  // SEEK_END
  seek_result = file.ops->Seek(&file, -3, SeekWhence::kEnd);
  EXPECT_TRUE(seek_result.has_value());
  EXPECT_EQ(seek_result.value(), strlen(data) - 3);
}

// 测试读取目录
TEST_F(RamFsTest, ReadDirectory) {
  Inode* root = ramfs_.GetRootInode();
  ASSERT_NE(root, nullptr);

  // 创建一些文件和目录
  (void)root->ops->Create(root, "file1.txt", FileType::kRegular);
  (void)root->ops->Create(root, "file2.txt", FileType::kRegular);
  (void)root->ops->Mkdir(root, "dir1");

  // 创建 File 对象用于 readdir
  File dir_file;
  dir_file.inode = root;
  dir_file.offset = 0;
  dir_file.ops = ramfs_.GetFileOps();

  DirEntry entries[16];
  auto readdir_result = dir_file.ops->ReadDir(&dir_file, entries, 16);
  EXPECT_TRUE(readdir_result.has_value());

  // 应该至少有 . 和 .. 加上我们创建的文件和目录
  size_t count = readdir_result.value();
  EXPECT_GE(count, 2u);  // 至少包含 . 和 ..
}

// 测试重复创建文件
TEST_F(RamFsTest, CreateDuplicateFile) {
  Inode* root = ramfs_.GetRootInode();
  ASSERT_NE(root, nullptr);

  // 创建文件
  auto create_result =
      root->ops->Create(root, "duplicate.txt", FileType::kRegular);
  EXPECT_TRUE(create_result.has_value());

  // 重复创建应该失败
  create_result = root->ops->Create(root, "duplicate.txt", FileType::kRegular);
  EXPECT_FALSE(create_result.has_value());
}

// 测试删除非空目录
TEST_F(RamFsTest, RmdirNonEmpty) {
  Inode* root = ramfs_.GetRootInode();
  ASSERT_NE(root, nullptr);

  // 创建目录
  auto mkdir_result = root->ops->Mkdir(root, "nonempty_dir");
  ASSERT_TRUE(mkdir_result.has_value());

  Inode* dir = mkdir_result.value();

  // 在目录中创建文件
  (void)dir->ops->Create(dir, "file_inside.txt", FileType::kRegular);

  // 尝试删除非空目录应该失败
  auto rmdir_result = root->ops->Rmdir(root, "nonempty_dir");
  EXPECT_FALSE(rmdir_result.has_value());
}

// 测试删除不存在的文件
TEST_F(RamFsTest, UnlinkNonExistent) {
  Inode* root = ramfs_.GetRootInode();
  ASSERT_NE(root, nullptr);

  auto unlink_result = root->ops->Unlink(root, "nonexistent.txt");
  EXPECT_FALSE(unlink_result.has_value());
}

// 测试 Sync（ramfs 应该立即返回成功）
TEST_F(RamFsTest, Sync) {
  auto sync_result = ramfs_.Sync();
  EXPECT_TRUE(sync_result.has_value());
}
