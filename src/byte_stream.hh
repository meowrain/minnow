#pragma once

#include <algorithm>
#include <cstdint>
#include <deque>
#include <iostream>
#include <string>
#include <string_view>
class Reader;
class Writer;

class ByteStream
{
public:
  explicit ByteStream( uint64_t capacity );

  // 访问 ByteStream 的 Reader 和 Writer 接口的辅助函数（已提供）
  Reader& reader();
  const Reader& reader() const;
  Writer& writer();
  const Writer& writer() const;

  // 发出流发生错误的信号。
  void set_error() { error_ = true; };
  // 返回流是否有错误
  bool has_error() const { return error_; };

protected:
  // 请将任何附加状态添加到此处的 ByteStream，而不是添加到 Writer 和 Reader 接口。
  // 使用deque双端队列存储字节
  std::deque<char> buffer_ {};
  // 容量
  uint64_t capacity_;
  // 错误默认初始化为false
  bool error_ {};
  // 流默认状态为false，也就是打开状态
  bool closed_ {};
  // 写入的字节数
  uint64_t bytes_written_ {};
  // 读取的字节数
  uint64_t bytes_read_ {};
};

class Writer : public ByteStream
{
public:
  // 将数据推送到流中，但仅限于可用容量允许的数量。
  void push( std::string data );
  // 指示流已到达结尾。不会再写更多的了。
  void close();

  // 返回流是否已关闭
  bool is_closed() const;
  // 现在可以将多少字节推送到流中？
  uint64_t available_capacity() const;
  // 累计推送到流的总字节数
  uint64_t bytes_pushed() const;
};

class Reader : public ByteStream
{
public:
  // 查看缓冲区中的下一个字节
  std::string_view peek() const;
  // 从缓冲区中删除 `len` 字节
  void pop( uint64_t len );

  // 流是否已完成（关闭并完全弹出）？
  bool is_finished() const;
  // 当前缓冲的字节数（推送且未弹出）
  uint64_t bytes_buffered() const;
  // 从流中累积弹出的字节总数
  uint64_t bytes_popped() const;
};

/*
 * read: A (provided) helper function thats peeks and pops up to `len` bytes
 * from a ByteStream Reader into a string;
 */
void read( Reader& reader, uint64_t len, std::string& out );
