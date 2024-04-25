#pragma once
#include "byte_stream.hh"
#include <set>
#include <utility>
/**
 * 这个实验室检查点（Lab
Checkpoint）是CS144课程《计算机网络导论》的一部分，主题是计算机网络，特别是关于传输控制协议（TCP）的实现。以下是主要要求和实现目标的概述：

1.
**目的和背景**：学生将通过实现TCP来学习如何在不可靠的数据报网络之上提供可靠的字节流服务。TCP是互联网上最广泛使用的协议之一，它能够在数据可能丢失、乱序、被篡改或重复的情况下，提供可靠的字节流。

2. **起步**：学生需要使用Minnow库来开始他们的TCP实现，这个库也是之前检查点（Checkpoint
0）中使用的。学生需要确保他们的代码提交到了Git仓库，并且通过特定的Git命令来获取最新的实验室作业代码。

3. **构建和编译**：学生需要设置构建系统并编译源代码。

4. **编写文档**：学生需要打开并开始编辑`writeups/check1.md`文件，这是实验室报告的模板，将包含在提交中。

5.
**实现TCP接收器**：学生将实现一个TCP接收器模块，该模块接收数据报并将其转换为可靠的字节流，以便应用程序可以从套接字读取。

6.
**数据结构设计**：学生需要设计并实现一个名为`Reassembler`的数据结构，它负责接收子字符串（substrings），并将它们重新组装成原始的连续字节流。每个字节流中的字节都有一个唯一的索引，从零开始递增。

7. **Reassembler接口**：`Reassembler`类需要实现以下接口：
   - `insert(uint64_t first_index, std::string data, bool is_last_substring)`：插入一个新的子字符串到字节流中。
   - `uint64_t bytes_pending() const`：返回`Reassembler`本身存储了多少字节。
   - `Reader& reader()`：访问输出流的读取器。

8. **内部存储**：`Reassembler`需要内部存储三种类别的数据：
   - 下一个在流中的字节。
   - 可以放入流中但因为前面的字节未知而不能立即写入的字节。
   - 超出流容量的字节，这些字节应该被丢弃。

9.
**效率和数据结构**：学生需要考虑数据结构的选择，以确保实现的效率。提供了一个性能基准，要求实现的`Reassembler`至少能达到0.1
Gbit/s的处理速度。

10.
**开发和调试建议**：学生可以使用特定的命令来测试代码，并被鼓励保持代码的可读性，使用合理的命名约定，并使用防御性编程。

11.
**提交要求**：学生需要在提交中只修改指定的`.hh`和`.cc`文件，并在这些文件中添加必要的私有成员，但不能改变任何类的公共接口。

12. **报告**：学生需要写一个报告，描述他们的代码结构和设计选择，遇到的挑战，以及任何剩余的bug或未处理的边缘情况。

这个实验室检查点旨在通过实践来加深学生对TCP协议和网络编程的理解。
 *
*/
class Reassembler
{
public:
  // Construct Reassembler to write into given ByteStream.
  explicit Reassembler( ByteStream&& output ) : output_( std::move( output ) ) {};

  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring );

  // How many bytes are stored in the Reassembler itself?
  uint64_t bytes_pending() const;

  // Access output stream reader
  Reader& reader() { return output_.reader(); }
  const Reader& reader() const { return output_.reader(); }

  // Access output stream writer, but const-only (can't write from outside)
  const Writer& writer() const { return output_.writer(); }

private:
  ByteStream output_; // the Reassembler writes to this ByteStream
  struct Seg
  {
    uint64_t first_index;// 表示该数据段在原始字节流中的起始索引
    std::string data;
    bool operator<( const Seg& other ) const { return first_index < other.first_index; }
    Seg( uint64_t f, std::string  d ) : first_index( f ), data(std::move( d )) {};
  };
  std::set<Seg> segments_ {};
  uint64_t bytes_waiting_ {};
  uint64_t first_unpoped_index_ {};
  uint64_t first_unassembled_index_ {};//当前重组器应该处理的字节流中的下一个字节的索引
  uint64_t final_index_ = INT64_MAX;
  void check_push();
};
