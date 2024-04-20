#pragma once

#include <cstddef>
#include <limits>
#include <memory>
#include <vector>

// 文件描述符的引用计数句柄
/**
 * 这个短语描述了一种处理文件描述符的方法。在操作系统编程中，文件描述符是用于标识已打开文件或其他I/O资源的整数值。
 * 而“reference-counted
 * handle”则指的是一个引用计数句柄，用于管理对资源的引用计数。这种方法可以确保资源在不再被引用时被正确释放，从而避免内存泄漏或资源泄漏。
 * 因此，“A reference-counted handle to a file descriptor”
 * 可以理解为一种机制，它提供了一种管理文件描述符的方式，确保在不再需要时正确释放相关资源，同时还能够在需要时共享对资源的引用。
 *
 */
class FileDescriptor
{
  // FDWrapper：内核文件描述符的句柄。
  // FileDescriptor 对象包含 FDWrapper 的 std::shared_ptr。
  class FDWrapper
  {
  public:
    int fd_;                    // 内核返回的文件描述符编号
    bool eof_ = false;          // 指示 FDWrapper：：fd_ 是否位于 EOF 的标志
    bool closed_ = false;       // 指示 FDWrapper：：fd_ 是否已关闭的标志
    bool non_blocking_ = false; // 指示 FDWrapper：：fd_ 是否为非阻塞的标志
    unsigned read_count_ = 0;   // FDWrapper：：fd_ 被读取的次数
    unsigned write_count_ = 0;  // DWrapper：：fd_ 被写入的次数

    // 根据内核返回的文件描述符编号构造
    explicit FDWrapper( int fd );
    // 销毁时关闭文件描述符,析构函数
    ~FDWrapper();
    // 在 FDWrapper::fd_ 上调用 [close(2)](\ref man2::close)
    void close();

    template<typename T>
    T CheckSystemCall( std::string_view s_attempt, T return_value ) const;

    // FDWrapper 无法复制或移动，关闭其默认的复制构造
    FDWrapper( const FDWrapper& other ) = delete;
    FDWrapper& operator=( const FDWrapper& other ) = delete;
    FDWrapper( FDWrapper&& other ) = delete;
    FDWrapper& operator=( FDWrapper&& other ) = delete;
  };

  // 共享 FDWrapper 的引用计数句柄
  std::shared_ptr<FDWrapper> internal_fd_;

  // 用于复制 FileDescriptor 的私有构造函数（增加引用计数）
  explicit FileDescriptor( std::shared_ptr<FDWrapper> other_shared_ptr );

protected:
  // 为 read() 分配的缓冲区大小
  static constexpr size_t kReadBufferSize = 16384;

  void set_eof() { internal_fd_->eof_ = true; }
  // 增加读取计数
  void register_read() { ++internal_fd_->read_count_; }   
  // 增加写入次数
  void register_write() { ++internal_fd_->write_count_; } 

  template<typename T>
  T CheckSystemCall( std::string_view s_attempt, T return_value ) const;

public:
  // 根据内核返回的文件描述符编号构造
  explicit FileDescriptor( int fd );

  // 释放 std::shared_ptr;当引用计数变为零时，FDWrapper 析构函数调用 close()。
  ~FileDescriptor() = default;

  // 读入`buffer`
  void read( std::string& buffer );
  void read( std::vector<std::string>& buffers );

  // 尝试写入缓冲区
  // 返回写入的字节数
  size_t write( std::string_view buffer );
  size_t write( const std::vector<std::string_view>& buffers );
  size_t write( const std::vector<std::string>& buffers );

//关闭底层文件描述符
  void close() { internal_fd_->close(); }

//显式复制 FileDescriptor，增加 FDWrapper 引用计数
  FileDescriptor duplicate() const;

//设置阻塞（true）或非阻塞（false）
  void set_blocking( bool blocking );

//文件大小
/**
 * `off_t`是一个在C和C++中用于表示文件偏移量（file offset）的类型。
 * 它通常用于文件操作中，用来指示文件中的位置或偏移量。在不同的操作系统和平台上，
 * `off_t`的大小可能会有所不同，但通常会被定义为一个整数类型（如`long`或`long long`）。
在Linux和Unix系统中，`off_t`通常被定义为`long`类型，它用于表示文件中的偏移量。在文件操作中，`off_t`常用于`lseek`函数来移动文件指针到指定的偏移量位置。
*/
  off_t size() const;

  // FDWrapper accessors
  int fd_num() const { return internal_fd_->fd_; }//底层描述符编号
  bool eof() const { return internal_fd_->eof_; }//EOF 标志状态
  bool closed() const { return internal_fd_->closed_; }//关闭标志状态
  unsigned int read_count() const { return internal_fd_->read_count_; }//读取次数
  unsigned int write_count() const { return internal_fd_->write_count_; }//写入次数

//复制/移动构造函数/赋值运算符
//FileDescriptor 可以移动，但不能隐式复制（参见duplicate()）
  FileDescriptor( const FileDescriptor& other ) = delete;//禁止复制构造
  FileDescriptor& operator=( const FileDescriptor& other ) = delete;//禁止复制赋值
  FileDescriptor( FileDescriptor&& other ) = default;//允许移动构造
  FileDescriptor& operator=( FileDescriptor&& other ) = default;//允许移动赋值
};
