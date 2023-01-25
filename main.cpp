#include <cstdint>
#include <exception>
#include <iostream>
#include <string_view>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/write.hpp>

void process_request(boost::asio::io_context &io_context,
                     boost::asio::ip::tcp::socket &&tcp_stream,
                     boost::asio::yield_context yield) {
  constexpr std::string_view data = "HTTP/1.1 200 OK\r\n"
                                    "Content-Length: 2\r\n"
                                    "Content-Type: text/plain\r\n\r\n"
                                    "OK";
  boost::asio::async_write(tcp_stream, boost::asio::buffer(data),
                           boost::asio::transfer_all(), yield);
}

void main_coroutine(boost::asio::io_context &io_context,
                    boost::asio::yield_context yield) {
  const auto host = boost::asio::ip::make_address("127.0.0.1");
  const std::uint16_t port = 5000;
  const auto tcp_listen_addr = boost::asio::ip::tcp::endpoint{host, port};
  auto tcp_listener = boost::asio::ip::tcp::acceptor{io_context};
  tcp_listener.open(tcp_listen_addr.protocol());
  tcp_listener.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  tcp_listener.bind(tcp_listen_addr);
  tcp_listener.listen();
  std::clog << "Listening on " << tcp_listener.local_endpoint() << std::endl;
  while (true) {
    boost::system::error_code ec;
    auto tcp_stream = boost::asio::ip::tcp::socket(io_context);
    tcp_listener.async_accept(tcp_stream, yield[ec]);
    if (ec) {
      std::cerr << "Error in tcp_listener.accept():" << ec.what();
      continue;
    }
    std::clog << "Incoming connection from " << tcp_stream.remote_endpoint()
              << "/TCP" << std::endl;
    boost::asio::spawn(
        io_context, [&io_context, tcp_stream = std::move(tcp_stream)](
                        boost::asio::yield_context yield) mutable {
          try {
            process_request(io_context, std::move(tcp_stream), yield);
          } catch (const std::exception &ex) {
            std::cerr << "Error in process_request: " << ex.what();
          }
        });
  }
}

int main() {
  boost::asio::io_context io_context;
  boost::asio::spawn(io_context, [&](boost::asio::yield_context yield) {
    try {
      main_coroutine(io_context, yield);
    } catch (const std::exception &ex) {
      std::cerr << "main_coroutine: " << ex.what() << std::endl;
    }
  });
  io_context.run();
}
