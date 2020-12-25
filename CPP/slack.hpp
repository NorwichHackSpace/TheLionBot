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
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/system_executor.hpp>
#include <boost/asio/strand.hpp>

#include "root_certificates.hpp" //Imported from Boost examples, might not be up to date.

#include <rapidjson/reader.h>
#include <rapidjson/document.h>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace slack {

//slack.cpp
std::string HTTP( std::string call ) ;
std::string usertoname( std::string user ) ;

extern rapidjson::Document format; //Set by TheLionBot.cpp, hopefully.

//jokes.cpp
std::string xmas_joke();

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
	void send  (std::string message);
};

// Report a failure
void fail(beast::error_code ec, char const* what) ;

// Sends a WebSocket message and prints the response
class ws_session : public boost::enable_shared_from_this<ws_session>
{
	tcp::resolver resolver_;
	websocket::stream<
	beast::ssl_stream<beast::tcp_stream>> ws_;
	beast::flat_buffer buffer_;
	std::string host_;
	std::string path_;

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
	{ }

	void send(boost::shared_ptr<std::string const> const& ss)
	{
		net::post(
				ws_.get_executor(),
				beast::bind_front_handler(
						&ws_session::do_write,
						shared_from_this(),
						ss));
	}

	void do_write (boost::shared_ptr<std::string const> const& ss) {
		// Always add to queue
		queue_.push_back(ss);


		if ( !connected_ ) { //Queue messages until connected
			return;
		}

		// Are we already writing?
		if( sending_  ) {
			return;
		}
		sending_ = true;

		ws_.async_write(
				net::buffer(*queue_.front()),
				beast::bind_front_handler(
						&ws_session::on_write,
						shared_from_this()));
	}

	void on_write( beast::error_code ec, std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		if(ec)
			return fail(ec, "write");

		// Remove the string from the queue
		queue_.erase(queue_.begin());

		// Send the next message if any
		if( !queue_.empty() ) {
			ws_.async_write(
					net::buffer(*queue_.front()),
					beast::bind_front_handler(
							&ws_session::on_write,
							shared_from_this()));
		} else {
			sending_ = false;
		}

	}

	// Start the asynchronous operation
	void do_start( std::string host, char const* port, std::string path)
	{
		state_->join(this);  // Add this session to the list of active sessions
		// Save these for later
		host_ = host;
		path_ = path;
		// Look up the domain name
		resolver_.async_resolve(
				host,
				port,
				beast::bind_front_handler(
						&ws_session::on_resolve,
						shared_from_this()));
	}

	void do_start( std::string host, char const* port)
	{
		state_->join(this);  // Add this session to the list of active sessions
		// Save these for later
		host_ = host;
		path_ = "/";
		// Look up the domain name
		resolver_.async_resolve(
				host,
				port,
				beast::bind_front_handler(
						&ws_session::on_resolve,
						shared_from_this()));
	}

	void do_listen()
	{
		ws_.async_read(
				buffer_,
				beast::bind_front_handler(
						&ws_session::on_read,
						shared_from_this()));
	}

	void on_resolve( beast::error_code ec, tcp::resolver::results_type results)
	{
		if(ec)
			return fail(ec, "resolve");

		// Set a timeout on the operation
		beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

		// Make the connection on the IP address we get from a lookup
		beast::get_lowest_layer(ws_).async_connect(
				results,
				beast::bind_front_handler(
						&ws_session::on_connect,
						shared_from_this()));
	}

	void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
	{
		if(ec)
			return fail(ec, "connect");

		// Update the host_ string. This will provide the value of the
		// Host HTTP header during the WebSocket handshake.
		// See https://tools.ietf.org/html/rfc7230#section-5.4
		host_ += ':' + std::to_string(ep.port());

		// Set a timeout on the operation
		beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

		// Set SNI Hostname (many hosts need this to handshake successfully)
		if(! SSL_set_tlsext_host_name(
				ws_.next_layer().native_handle(),
				host_.c_str()))
		{
			ec = beast::error_code(static_cast<int>(::ERR_get_error()),
					net::error::get_ssl_category());
			return fail(ec, "connect");
		}

		// Perform the SSL handshake
		ws_.next_layer().async_handshake(
				ssl::stream_base::client,
				beast::bind_front_handler(
						&ws_session::on_ssl_handshake,
						shared_from_this()));
	}

	void on_ssl_handshake(beast::error_code ec)
	{
		if(ec)
			return fail(ec, "ssl_handshake");

		// Turn off the timeout on the tcp_stream, because
		// the websocket stream has its own timeout system.
		beast::get_lowest_layer(ws_).expires_never();

		// Set suggested timeout settings for the websocket
		ws_.set_option(
				websocket::stream_base::timeout::suggested(
						beast::role_type::client));

		// Set a decorator to change the User-Agent of the handshake
		ws_.set_option(websocket::stream_base::decorator(
				[](websocket::request_type& req)
				{
			req.set(http::field::user_agent,
					std::string(BOOST_BEAST_VERSION_STRING) +
					"websocket-client-async-ssl");
				}));

		// Perform the websocket handshake
		ws_.async_handshake(host_, path_,
				beast::bind_front_handler(
						&ws_session::on_handshake,
						shared_from_this()));
	}

	void on_handshake(beast::error_code ec)
	{
		if(ec)
			return fail(ec, "handshake");

		ws_.async_read(
				buffer_,
				beast::bind_front_handler(
						&ws_session::on_read,
						shared_from_this()));
	}


	void on_read( beast::error_code ec, std::size_t bytes_transferred)
	{

		boost::ignore_unused(bytes_transferred);

		if(ec)
			return fail(ec, "read");

		std::string buf = beast::buffers_to_string(buffer_.data());
		buffer_.clear();

		rapidjson::Document slackRead;
		slackRead.Parse( buf.c_str() );

		std::cout << "READ: " << buf << std::endl << std::endl;

		if ( slackRead.HasMember("type") && !slackRead.HasMember("subtype") && slackRead["type"] == "message" ) { //Simple first message received
			// Get the strings
			std::string channel = slackRead["channel"].GetString();
			std::string user = slackRead["user"].GetString();
			std::string text = slackRead["text"].GetString();
			std::string event = slackRead["event_ts"].GetString();
			// Process the strings
			if (channel == "CUQV9AGBW" && user == "CMFJQ7NNB") { //Only for Dootbot messages in the #door-status channel
				slack::slackDoorbotHandle( text, user, channel, event );
			} else {
				text = slack::slackMsgHandle ( text, user, channel, event ); //Split into message.cpp
				if (text != "") {
					std::cout << "WRITE: " << text << std::endl << std::endl;
					state_->send(text); //Add message to queue
				}
			}
		} else if ( slackRead.HasMember("type") && !slackRead.HasMember("subtype") && slackRead["type"] == "hello" ) {
			std::cout << "HELLO."  << std::endl;
			connected_ = true; //Messages get queued untl Slack confirms that it is ready
			state_->send(" { \"channel\" : \"D81AQQPFT\" , \"text\" : \"Started build " __DATE__ " " __TIME__ "! :lion_face: \" , \"type\" : \"message\" }");
		} else if ( slackRead.HasMember("type") && !slackRead.HasMember("subtype") && slackRead["type"] == "goodbye" ) {
			// Close the WebSocket connection
			ws_.async_close(websocket::close_code::normal,
					beast::bind_front_handler(
							&ws_session::on_close,
							shared_from_this()));
		}

		ws_.async_read(
				buffer_,
				beast::bind_front_handler(
						&ws_session::on_read,
						shared_from_this()));
	}

	void on_close(beast::error_code ec)
	{
		if(ec)
			return fail(ec, "close");

		// If we get here then the connection is closed gracefully

		// The make_printable() function helps print a ConstBufferSequence
		std::cout << beast::make_printable(buffer_.data()) << std::endl;
	}
};

//Do not add code below this line
#endif /* CPP_THELIONBOT_HPP_ */
