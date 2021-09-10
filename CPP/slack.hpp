/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : TheLionBot.cpp
 Authors     : Alan Percy Childs
 Version     :
 *******************************************************************************/
/*
   Only modify this file to include
    - function definitions (prototypes)
    - include files
    - extern variable definitions
   In the appropriate sections
 */

#ifndef SLACK_HPP_
#define SLACK_HPP_

#include <iostream> //Only for cout
#include <cstdlib>
#include <functional>
#include <memory>
#include <string>
#include <unordered_set>
#include <mutex>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/system_executor.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <rapidjson/reader.h>
#include <rapidjson/document.h>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace slack { //The simple stuff

	//slack.cpp
	std::string HTTP( std::string type, std::string call ) ;
	std::string RTM( std::string call ) ;

	std::string usertoname( std::string user ) ;
	std::string reaction( std::string channel, std::string timestamp, std::string emoji ) ;

	extern rapidjson::Document startJSON; //Set by TheLionBot.cpp, hopefully.

	//jokes.cpp
	std::string joke();
	std::string joke_xmas();

	//doorbot.cpp
	std::string doorstatus();

	//whoami.cpp
	std::string whoami( std::string user );

	//whoisin.cpp
	std::string amendlog( std::string text, std::string user );
	std::string amendlog( unsigned int population );
	std::string occupancy();

	std::string slackMsgHandle( std::string text, std::string user, std::string channel, std::string event_ts );
	void slackDoorbotHandle( std::string text, std::string user, std::string channel, std::string event_ts );

}

//Handle responses
class response {
	private:
		int lastmsg = -1;
	public:
		int random(int size) {
			int msgnum = (rand() % size);
			while (msgnum == lastmsg) {
				msgnum = (rand() % size);
			}
			lastmsg = msgnum;
			return msgnum;
		};
		int sequence(int size) {
			if (++lastmsg > size) { lastmsg = 0; }
			return lastmsg;
		};
};

class ws_session; // Forward declaration

class shared_state
{
		std::string const id_;

		// This mutex synchronizes all access to sessions_
		std::mutex mutex_;

		// Keep a list of all the connected clients
		std::unordered_set<ws_session*> sessions_;

	public:
		explicit shared_state(std::string id)
		: id_(std::move(id))
		{ }
		;

		std::string const& id() const noexcept
		{
			return id_;
		}

		void join  (ws_session* shared_session);
		void leave (ws_session* shared_session);
		void restart(ws_session* shared_session);
		void send  (std::string message);
};

extern const boost::detail::sp_if_not_array<shared_state>::type slackthread;

// Report a failure
void fail(beast::error_code ec, char const* what) ;

// Sends a WebSocket message and prints the response
class ws_session : public boost::enable_shared_from_this<ws_session>  //The complicated stuff
{
		tcp::resolver resolver_;
		websocket::stream<
		beast::ssl_stream<beast::tcp_stream>> ws_;
		beast::flat_buffer buffer_;
		std::string host_;
		std::string path_;
		const char * port_ = "443";
		net::io_context* ioc_;
		boost::shared_ptr<shared_state> state_;
		std::vector<boost::shared_ptr<std::string const>> queue_; //Stuff that needs sending
		bool connected_ = false;
		bool sending_ = false;

	public:

		ws_session(
				net::io_context& ioc
				, ssl::context& ctx
				, boost::shared_ptr<shared_state> const& state
			)
			: resolver_(ioc)
			, ws_(ioc, ctx)
			, state_(state)
		{ ioc_ = &ioc; }

		void send(boost::shared_ptr<std::string const> const& ss);
		void do_write (boost::shared_ptr<std::string const> const& ss);
		void on_write( beast::error_code ec, std::size_t bytes_transferred);
		void on_read( beast::error_code ec, std::size_t bytes_transferred); //Broken out in CPP file
		// Start the asynchronous operation
		void do_start( );
		void do_listen( );
		void on_resolve( beast::error_code ec, tcp::resolver::results_type results );
		void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
		void on_ssl_handshake(beast::error_code ec);
		void on_handshake(beast::error_code ec);
		//
		void on_close(beast::error_code ec);

};

//Do not add code below this line
#endif /* CPP_THELIONBOT_HPP_ */
