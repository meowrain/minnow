#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  //  const Writer& writers = writer();
  // Your code here
  // 计算当前数据段的最后一个索引
  uint64_t last_index = first_index + data.size();
  // 计算当前重组器未处理的起始索引
  uint64_t first_unacceptable = first_unassembled_index_ + output_.writer().available_capacity();
  // 如果是最后一个子字符串，那么就更新final_index_
  if ( is_last_substring ) {
    final_index_ = last_index;
  }
  // 如果当前数据段的起始索引小于当前重组器应该处理的字节流中的下一个字节的索引
  if ( first_index < first_unassembled_index_ ) {
    // 检查当前数据段是否完全位于已处理的索引范围内：
    // 如果当前数据段的最后一个索引（first_index + data.size()）小于或等于
    // 当前重组器应该处理的字节流中的下一个字节的索引（first_unassembled_index_），
    // 这表明该数据段以及其所有字节都已经是有序的，并且可以安全地推送到输出流中。
    // 因此，调用check_push尝试推送数据，并结束当前insert函数的执行。
    if ( first_index + data.size() <= first_unassembled_index_ ) {
      check_push();
      return;
    } else {
      // 如果>的话，说明超出去的部分是不能放进去的，我们只需要把已经有序的部分放进去就好了
      data.erase( 0, first_unassembled_index_ - first_index );
      first_index = first_unassembled_index_;
    }
  }
  if ( first_unacceptable <= first_index ) {
    //如果第一个不能接受的索引<=要插入的数据段的起始索引，那就不用插入了，直接return
    return;
  }
  if ( last_index > first_unacceptable ) {
    //如果这个数据段的最后一个索引>第一个不能接受的索引，那我们只需要取从first_index到first_unacceptable的数据段就可以了，剩下超出去的就没必要留着了。因为已经超出capacity了
    data.erase( first_unacceptable - first_index );
  }

  if ( !segments_.empty() ) {
    //如果数据段set不为空
    auto cur = segments_.lower_bound( Seg( first_index, "" ) );//快速找到第一个起点大于等于要插入字串的字串
    if ( cur != segments_.begin() ) {
      //如果cur不是set集合中的第一个元素
      cur--;//cur向前移动一位,指向第一个起始索引小于first_index的数据段
      if ( cur->first_index + cur->data.size() > first_index ) {
        //如果前一个数据段的最后一个字节的索引大于first_index
        //那么就从当前要插入的数据段中删除前一个数据段已经覆盖的部分 ,可以看writeups中的check1.drawio文件
        data.erase( 0, cur->first_index + cur->data.size() - first_index );
        first_index += cur->first_index + cur->data.size() - first_index;
      }
    }
    
    cur = segments_.lower_bound( Seg( first_index, "" ) );
    // 再次使用 lower_bound 找到第一个起始索引大于等于 first_index 的数据段
    while ( cur != segments_.end() && cur->first_index < last_index ) {
      // 如果当前数据段完全在新插入的数据段范围内
      if ( cur->first_index + cur->data.size() <= last_index ) {
        // 如果当前数据段完全在新插入的数据段范围内
        // 减少bytes_wating的计数
        bytes_waiting_ -= cur->data.size();
        //从segments中删除这个数据段
        segments_.erase( cur );
        //重新定位cur到新的起始索引处
        cur = segments_.lower_bound( Seg( first_index, "" ) );
      } else {
        //当前数据段部分重叠或完全在新插入的数据段外部，那就从新插入的数据段中删除重叠部分
        data.erase( cur->first_index - first_index );
        break;//退出循环
      }
    }
  }
  //全部处理完毕，把这个要插入的数据段插入到segments_中
  segments_.insert( Seg( first_index, data ) );
  bytes_waiting_ += data.size();
  //推送到bytes_stream中
  check_push();
  //  (void)first_index;
  //  (void)data;
  //  (void)is_last_substring;
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return bytes_waiting_;
}

//Reassembler::check_push() 函数是 Reassembler 类的一个成员函数，其主要目的是将已经按顺序准备好的数据段推送到 ByteStream 的写端
void Reassembler::check_push()
{
  /**
   * 循环条件：while ( !segments_.empty() ) 创建了一个循环，该循环将持续进行，直到 segments_ 集合为空。segments_
是一个 set 集合，存储了待重组的数据段。

获取第一个段：auto seg = segments_.begin(); 获取 segments_ 中第一个元素的迭代器。由于 set
是有序的，第一个元素将是具有最小 first_index 的数据段。

  检查索引：if ( seg->first_index == first_unassembled_index_ ) 检查这个数据段的 first_index 是否恰好是
first_unassembled_index_，即重组器当前应该处理的字节流的索引。

    推送数据：如果索引匹配，output_.writer().push( seg->data ); 将数据段的内容 seg->data 推送到 ByteStream 的写端。

  保存迭代器：auto tmp = seg; 保存当前 seg 迭代器的副本，因为接下来的操作可能会使 seg 迭代器失效。

  更新索引和计数器：

    first_unassembled_index_ += seg->data.size(); 更新
first_unassembled_index_，它指向下一个需要处理的数据字节的索引。 bytes_waiting_ -= seg->data.size();
减少等待重组的字节数。 删除段：segments_.erase( tmp ); 从 segments_ 集合中删除已经推送的数据段。

  关闭输出：如果 first_unassembled_index_ 已经达到或超过了 final_index_（表示所有数据段都已重组完毕），则调用
output_.writer().close(); 关闭 ByteStream 的写端。

  循环终止条件：else { break; } 如果当前数据段的 first_index 不匹配
first_unassembled_index_，说明没有更多的数据段可以推送，因此退出循环。
   */
  while ( !segments_.empty() ) {
    auto seg = segments_.begin();
    if ( seg->first_index == first_unassembled_index_ ) {
      output_.writer().push( seg->data );
      auto tmp = seg;
      first_unassembled_index_ += seg->data.size();
      bytes_waiting_ -= seg->data.size();
      segments_.erase( tmp );
      if ( first_unassembled_index_ >= final_index_ ) {
        output_.writer().close();
      }
    } else {
      break;
    }
  }
}