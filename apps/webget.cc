#include "socket.hh"

#include <cstdlib>
#include <iostream>
#include <span>
#include <string>

using namespace std;

void get_URL( const string& host, const string& path )
{
  auto sc = TCPSocket();
  Address addr = Address( host, "http" );
  sc.connect( addr );
  sc.write( "GET " + path + " " + "HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n" );
  sc.shutdown( SHUT_WR );
  std::string str;
  while ( !sc.eof() ) {
    sc.read( str );
    std::cout << str;
  }
  sc.close();
  // cerr << "Function called: get_URL(" << host << ", " << path << ")\n";
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc <= 0 ) {
      abort(); // For sticklers: don't try to access argv[0] if argc <= 0.
    }

    auto args = span( argv, argc );

    // The program takes two command-line arguments: the hostname and "path" part of the URL.
    // Print the usage message unless there are these two arguments (plus the program name
    // itself, so arg count = 3 in total).
    if ( argc != 3 ) {
      cerr << "Usage: " << args.front() << " HOST PATH\n";
      cerr << "\tExample: " << args.front() << " stanford.edu /class/cs144\n";
      return EXIT_FAILURE;
    }

    // 获取命令行参数。
    const string host { args[1] };
    const string path { args[2] };

    // 调用学生编写的函数。
    get_URL( host, path );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
