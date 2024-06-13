
/**
 * @file singleton.hpp
 * @brief 单例模版
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-07-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-07-15<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#ifndef SIMPLEKERNEL_SRC_KERNEL_INCLUDE_SINGLETON_HPP_
#define SIMPLEKERNEL_SRC_KERNEL_INCLUDE_SINGLETON_HPP_

// 单例模板类
template <typename T>
class Singleton {
 public:
  // 获取单例实例的静态方法
  static T& GetInstance() {
    static T instance;
    return instance;
  }

  Singleton() = default;
  ~Singleton() = default;

 private:
  /// @name 构造/析构函数
  /// @{
  Singleton(const Singleton&) = delete;
  Singleton(Singleton&&) = delete;
  auto operator=(const Singleton&) -> Singleton& = delete;
  auto operator=(Singleton&&) -> Singleton& = delete;
  /// @}
};

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_SINGLETON_HPP_ */
