#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  // Your code here.
  return closed_;
}

void Writer::push( string data ) {
  //如果流已经关闭或者是有错误发生，那么就设置error为true，然后退出程序
  if ( closed_ || error_ ) {
    set_error();
    return;
  }

  if (data.length() > available_capacity()) {//如果插入数据长度大于可用的容量，那么就截断数据
    data = data.substr(0, available_capacity()); // 截断数据以适应可用容量，并修改data为截断后的数据
  }
  //在双端队列中插入数据，使用copy函数可实现拷贝string中的数据到双端队列buffer_中
  //std::back_inserter 是一个插入迭代器，它提供一种通用的方式来将元素插入到容器的末尾。在这个场景下，每当 std::copy 算法尝试通过迭代器写入一个元素时，
  //std::back_inserter 调用双端队列的 push_back 方法，将新元素添加到队列的末尾。
  std::copy( data.begin(), data.end(), std::back_inserter( buffer_ ) );
  //写入完成后，把写入的字节数加上data的大小
  bytes_written_ += data.size();
}

void Writer::close()
{
  closed_ = true;
  // Your code here.
}

uint64_t Writer::available_capacity() const
{

  // Your code here.
  uint64_t ava_capacity = capacity_ - buffer_.size();
  return ava_capacity;
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return bytes_written_;
}

bool Reader::is_finished() const
{
  // Your code here.
  bool res = closed_ && buffer_.empty();
  return res;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return bytes_read_;
}

string_view Reader::peek() const
{
  // Your code here.
  if ( !buffer_.empty() ) {
    return std::string_view( &buffer_.front(), 1 );
  }
  return std::string_view();
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  if ( len > buffer_.size() ) {
    len = buffer_.size();
  }
  buffer_.erase( buffer_.begin(), buffer_.begin() + len );
  bytes_read_ += len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buffer_.size();
}
