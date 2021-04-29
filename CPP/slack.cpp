/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : slack.cpp
 Authors     : Alan Percy Childs
 Version     :
 Wiki Page	 : https://wiki.norwichhackspace.org/index.php?title=Slack
 Disclaimer  : Any resemblance to actual robots would be really cool
*******************************************************************************/

#include "TheLionBot.hpp"
#include "slack.hpp"

#include <iostream>

using namespace std;

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = net::ssl;       // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


//------------------------------------------------------------------------------

void shared_state::join(ws_session* shared_session)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.insert(shared_session);
}

void shared_state::leave(ws_session* shared_session)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(shared_session);
}

/*
void shared_state::restart(ws_session* shared_session)
{
    std::lock_guard<std::mutex> lock(mutex_);
    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv12_client};
    load_root_certificates(ctx);
    boost::make_shared<ws_session>(
    		  ioc
      		, ctx
			, this->id
		)->do_start();
	sessions_.erase(shared_session);
}
*/

void shared_state::send(std::string message)
{
    // Put the message in a shared pointer so we can re-use it for each client
    auto const ss = boost::make_shared<std::string const>(std::move(message));
    // Make a local list of all the weak pointers representing
    // the sessions, so we can do the actual sending without
    // holding the mutex:
    std::vector<boost::weak_ptr<ws_session>> v;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        v.reserve(sessions_.size());
        for(auto p : sessions_)
            v.emplace_back(p->weak_from_this());
    }
    // For each session in our local list, try to acquire a strong
    // pointer. If successful, then send the message on that session.
    for(auto const& wp : v)
        if(auto sp = wp.lock())
            sp->send(ss);

}

/*
 * Websocket Session Code specific to Slack
 * Mostly because of the "Hello" response when ready (not on handshake) issue.
 *
 */


void ws_session::do_write (boost::shared_ptr<std::string const> const& ss) {
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

void ws_session::on_write( beast::error_code ec, std::size_t bytes_transferred)
{
	boost::ignore_unused(bytes_transferred);

	if(ec) {
		state_->leave(this);
		return fail(ec, "write");
	}

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

void ws_session::do_start() // Start the asynchronous operation
{
    /*
     * To establish a Websocket we first need to request a new token from Slack via rtc.start.
     * This is then used to form the URL in a new WSS request, instead of upgrading the connection.
     * Note that we could just use rtc.connect but by using rtc.start, as a bonus, we end up with a JSON
     * of user and channel details also.
     */

	slack::startJSON.Parse(slack::HTTP("start").c_str()); //Populate 'startJSON' with data from Slack Web API call 'start'
	LUrlParser::ParseURL slackWSurl = LUrlParser::ParseURL::parseURL(slack::startJSON["url"].GetString());
	if (!slackWSurl.isValid())
	{
		//TODO: Throw error instead of cout
		cout << "URL Parsing error: " << slackWSurl.errorCode_ << endl;
		return;
	}

	cout << "Scheme    : " << slackWSurl.scheme_ << endl;
	cout << "Host      : " << slackWSurl.host_ << endl;
	cout << "Path      : " << slackWSurl.path_ << endl;
	cout << endl;

	state_->join(this);  // Add this session to the list of active sessions

	//Copy ParseURL results to ws_session
    port_ = "443";
    path_ = "/" + slackWSurl.path_;
    host_ = slackWSurl.host_;
    BUGLINE
	//Look up the domain name
	resolver_.async_resolve(
			host_,
			port_,
			beast::bind_front_handler(
					&ws_session::on_resolve,
					shared_from_this()));
}

void ws_session::on_resolve( beast::error_code ec, tcp::resolver::results_type results)
{
	if(ec) {
		state_->leave(this);
		return fail(ec, "resolve");
	}
	BUGLINE
	// Set a timeout on the operation
	beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30)); // @suppress("Invalid arguments") // @suppress("Method cannot be resolved")

	// Make the connection on the IP address we get from a lookup
	beast::get_lowest_layer(ws_).async_connect( // @suppress("Invalid arguments") // @suppress("Method cannot be resolved")
			results,
			beast::bind_front_handler(
					&ws_session::on_connect,
					shared_from_this()));
}

void ws_session::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
{
	if(ec) {
		state_->leave(this);
		return fail(ec, "connect");
	}

	BUGLINE
	// Update the host_ string. This will provide the value of the
	// Host HTTP header during the WebSocket handshake.
	// See https://tools.ietf.org/html/rfc7230#section-5.4
	host_ += ':' + std::to_string(ep.port());
	// Set a timeout on the operation
	beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30)); // @suppress("Invalid arguments") // @suppress("Method cannot be resolved")

	// Set SNI Hostname (many hosts need this to handshake successfully)
	if(! SSL_set_tlsext_host_name(
			ws_.next_layer().native_handle(),
			host_.c_str()))
	{
		ec = beast::error_code(static_cast<int>(::ERR_get_error()),
				net::error::get_ssl_category());
		return fail(ec, "connect");
	}
	BUGLINE
	// Perform the SSL handshake
	ws_.next_layer().async_handshake(
			ssl::stream_base::client,
			beast::bind_front_handler(
					&ws_session::on_ssl_handshake,
					shared_from_this()));
}

void ws_session::on_ssl_handshake(beast::error_code ec)
{
	if(ec) {
		state_->leave(this);
		return fail(ec, "ssl_handshake");
	}
	BUGLINE
	// Turn off the timeout on the tcp_stream, because
	// the websocket stream has its own timeout system.
	beast::get_lowest_layer(ws_).expires_never(); // @suppress("Invalid arguments") // @suppress("Method cannot be resolved")
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
	BUGLINE
	// Perform the websocket handshake
	ws_.async_handshake(host_, path_,
			beast::bind_front_handler(
					&ws_session::on_handshake,
					shared_from_this()));
}

void ws_session::on_handshake(beast::error_code ec)
{
	if(ec) {
		state_->leave(this);
		return fail(ec, "handshake");
	}
	BUGLINE
	ws_.async_read(
			buffer_,
			beast::bind_front_handler(
					&ws_session::on_read,
					shared_from_this()));
}

void ws_session::do_listen()
{
	BUGLINE
	ws_.async_read(
			buffer_,
			beast::bind_front_handler(
					&ws_session::on_read,
					shared_from_this()));
}

void ws_session::send(boost::shared_ptr<std::string const> const& ss)
{
	net::post(
			ws_.get_executor(),
			beast::bind_front_handler(
					&ws_session::do_write,
					shared_from_this(),
					ss));
}

void ws_session::on_read( beast::error_code ec, std::size_t bytes_transferred)
{

	boost::ignore_unused(bytes_transferred);

	if(ec) {
		state_->leave(this);
		return fail(ec, "read");
	}
	BUGLINE
	std::string buf = beast::buffers_to_string(buffer_.data());
	buffer_.clear();

	rapidjson::Document slackRead;
	slackRead.Parse( buf.c_str() );

	std::cout << "READ: " << buf << std::endl << std::endl;

	if ( slackRead.HasMember("type") && !slackRead.HasMember("subtype") && slackRead["type"] == "message" ) { //Simple first message received
		//idle_timer.expires_from_now( IDLE_TIMEOUT ); //Something happened, so reset idle time.
		// Get the strings
		std::string channel = slackRead["channel"].GetString();
		std::string user = slackRead["user"].GetString();
		std::string text = slackRead["text"].GetString();
		std::string event = slackRead["ts"].GetString();

		//DEBUG STUFF
		if (text == "lion:reconnect" && channel == CHAN_LION_STATUS) {
			//state_->restart(this);
			//Close the WebSocket connection
			ws_.async_close(websocket::close_code::normal,
					beast::bind_front_handler(
							&ws_session::on_close,
							shared_from_this()));
		}
		//END DEBUG STUFF

		// Process the strings
		if (channel == CHAN_DOORSTATUS && user == USER_DOORBOT) { //Only for Dootbot messages in the #door-status channel
			slack::slackDoorbotHandle( text, user, channel, event );
		} else {
			text = slack::slackMsgHandle ( text, user, channel, event ); //Split into message.cpp
			if (text != "") {
				std::cout << "WRITE: " << text << std::endl << std::endl;
				state_->send(text); //Add message to queue
			}
		}
	} else if ( slackRead.HasMember("type") && !slackRead.HasMember("subtype") && slackRead["type"] == "hello" ) {
		connected_ = true; //Messages get queued until Slack confirms that it is ready
		if (!queue_.empty()) {
			ws_.async_write(
				net::buffer(*queue_.front()),
				beast::bind_front_handler(
						&ws_session::on_write,
						shared_from_this()));
		}
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

void ws_session::on_close(beast::error_code ec)
{
	if(ec)
		return fail(ec, "close");

	state_->leave(this);
	connected_ = false;

	// If we get here then the connection is closed gracefully

	// The make_printable() function helps print a ConstBufferSequence
	std::cout << "CLOSE: " << beast::make_printable(buffer_.data()) << std::endl;

}

//////////////////////////////////////////////////////////////////////

string slack::HTTP( string call ) {

	string slack_token = settings.GetValue("Slack", "Token", "Unset");
	if ( slack_token.empty() || slack_token == "Unset" ) {
	    std::cerr << "Error: " << "No Slack token set in conf file." << std::endl;
		settings.SetValue("Slack", "Token", "Unset");
		settings.SaveFile(INI_PATH); //Write new value to file
	    exit(EXIT_FAILURE);
	}
	string slack_fingerprint = settings.GetValue("Slack", "FINGERPRINT", "AC 95 5A 58 B8 4E 0B CD B3 97 D2 88 68 F5 CA C1 0A 81 E3 6E"); // if Slack changes their SSL (SHA1 fingerprint, you would need to update this:

    try
    {
        auto const host = "slack.com"; //I doubt Slack will change it's URL.
        auto const port = "443";
        string target = "/api/rtm." + call + "?token=" + slack_token;
        int version = 11; //HTTP Version. 1.1 Recommended.

        net::io_context ioc;

        //TODO: Avoid load_root_certificates every time function called
        ssl::context ctx{ssl::context::tlsv12_client};
        load_root_certificates(ctx);

        ctx.set_verify_mode(ssl::verify_peer);
        tcp::resolver resolver(ioc);
        beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);
        if(! SSL_set_tlsext_host_name(stream.native_handle(), host))
        {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            throw beast::system_error{ec};
        }

        auto const results = resolver.resolve(host, port);
        beast::get_lowest_layer(stream).connect(results); // @suppress("Invalid arguments") // @suppress("Method cannot be resolved")
        stream.handshake(ssl::stream_base::client);
        http::request<http::string_body> req{http::verb::get, target, version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Send the HTTP request to the remote host
        http::write(stream, req);
        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(stream, buffer, res);

    	string raw = boost::beast::buffers_to_string(res.body().data());

    	// Gracefully close the stream
        beast::error_code ec;
        stream.shutdown(ec);
        if(ec == net::error::eof)
        {
            ec = {};
        }
        if(ec)
            throw beast::system_error{ec};

        // If we get here then the connection is closed gracefully
    	return raw;
    }
    catch(std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return "Failed"; //TODO: Probably want to handle gracefully.
    }

}

string slack::usertoname( string user ) {
	// TODO: Check 'user' is a Slack user ID and not already converted
	// bool string match  -- name begins with 'U' has a number, all letters in caps, and is nine chars long -- , user
	//

	// else if needs converting
	for (rapidjson::Value::ConstValueIterator itr = slack::startJSON["users"].Begin(); itr != slack::startJSON["users"].End(); ++itr) { // Ok
	    if ( itr->HasMember("id") && (*itr)["id"].GetString() == user  ) { // Ok
	    	return (*itr)["profile"]["display_name_normalized"].GetString();
	    }
	}

	return user; //TODO: If we can't find, try making a new request.
}
