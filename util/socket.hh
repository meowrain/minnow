#pragma once

#include "address.hh"
#include "file_descriptor.hh"

#include <cstdint>
#include <functional>
#include <sys/socket.h>

//!\brief 网络套接字的基类（TCP、UDP 等）
//!\details Socket 通常通过子类使用。有关使用示例，请参阅 TCPSocket 和 UDPSocket。
class Socket : public FileDescriptor
{
private:
  // ！获取套接字连接到的本地或对等地址
  Address get_address( const std::string& name_of_function,
                       const std::function<int( int, sockaddr*, socklen_t* )>& function ) const;

protected:
  //! Construct via [socket(2)](\ref man2::socket)
  Socket( int domain, int type, int protocol = 0 );

  // ！从文件描述符构造。
  Socket( FileDescriptor&& fd, int domain, int type, int protocol = 0 );

  // ！ [getsockopt(2)](\ref man2::getsockopt) 的包装
  template<typename option_type>
  socklen_t getsockopt( int level, int option, option_type& option_value ) const;

  // ！ [setsockopt(2)](\ref man2::setsockopt) 的包装器
  template<typename option_type>
  void setsockopt( int level, int option, const option_type& option_value );

  void setsockopt( int level, int option, std::string_view option_val );

public:
  // ！使用 [bind(2)](\ref man2::bind) 将套接字绑定到指定地址，通常用于监听/接受
  void bind( const Address& address );

  // ！将套接字绑定到指定设备
  void bind_to_device( std::string_view device_name );

  // ！使用 [connect(2)](\ref man2::connect) 将套接字连接到指定的对等地址
  void connect( const Address& address );

  // ！通过 [shutdown(2)](\ref man2::shutdown) 关闭套接字
  void shutdown( int how );

  // ！使用 [getsockname(2)](\ref man2::getsockname) 获取套接字的本地地址
  Address local_address() const;
  // ！使用 [getpeername(2)](\ref man2::getpeername) 获取套接字的对等地址
  Address peer_address() const;

  // ！允许通过 [SO_REUSEADDR](\ref man7::socket) 更快地重用本地地址
  void set_reuseaddr();

  // ！检查错误（将在非阻塞套接字上看到）
  void throw_if_error() const;
};

class DatagramSocket : public Socket
{
  using Socket::Socket;

public:
  // ！接收数据报及其发送者的地址
  void recv( Address& source_address, std::string& payload );

  // ！发送数据报到指定地址
  void sendto( const Address& destination, std::string_view payload );

  // ！发送数据报到套接字的连接地址（必须首先调用connect()）
  void send( std::string_view payload );
};

//! A wrapper around [UDP sockets](\ref man7::udp)
class UDPSocket : public DatagramSocket
{
  // ！ \param[in] fd 是要构造的文件描述符
  explicit UDPSocket( FileDescriptor&& fd ) : DatagramSocket( std::move( fd ), AF_INET, SOCK_DGRAM ) {}

public:
  // ！默认：构造一个未绑定、未连接的 UDP 套接字
  UDPSocket() : DatagramSocket( AF_INET, SOCK_DGRAM ) {}
};

//! A wrapper around [TCP sockets](\ref man7::tcp)
class TCPSocket : public Socket
{
private:
  // ！ \brief 从 FileDescriptor 构造（由 Accept() 使用）
  // ！ \param[in] fd 是要构造的文件描述符
  explicit TCPSocket( FileDescriptor&& fd ) : Socket( std::move( fd ), AF_INET, SOCK_STREAM, IPPROTO_TCP ) {}

public:
  // ！默认值：构造一个未绑定、未连接的 TCP 套接字
  TCPSocket() : Socket( AF_INET, SOCK_STREAM ) {}

  // ！将套接字标记为侦听传入连接
  void listen( int backlog = 16 );

  // ！接受新的传入连接
  TCPSocket accept();
};

//! A wrapper around [packet sockets](\ref man7:packet)
class PacketSocket : public DatagramSocket
{
public:
  PacketSocket( const int type, const int protocol ) : DatagramSocket( AF_PACKET, type, protocol ) {}

  void set_promiscuous();
};

//! A wrapper around [Unix-domain stream sockets](\ref man7::unix)
class LocalStreamSocket : public Socket
{
public:
  //! Construct from a file descriptor
  explicit LocalStreamSocket( FileDescriptor&& fd ) : Socket( std::move( fd ), AF_UNIX, SOCK_STREAM ) {}
};

//! A wrapper around [Unix-domain datagram sockets](\ref man7::unix)
class LocalDatagramSocket : public DatagramSocket
{
  //! \param[in] fd is the FileDescriptor from which to construct
  explicit LocalDatagramSocket( FileDescriptor&& fd ) : DatagramSocket( std::move( fd ), AF_UNIX, SOCK_DGRAM ) {}

public:
  //! Default: construct an unbound, unconnected socket
  LocalDatagramSocket() : DatagramSocket( AF_UNIX, SOCK_DGRAM ) {}
};
