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
#include "Passwords.h"

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
ws_session::ws_session(
		net::io_context& ioc,
		ssl::context& ctx,
		std::shared_ptr<shared_state> const& state)
	: resolver_(ioc)
	, ws_(ioc, ctx)
	, state_(state)
{
}
*/

string slack::HTTP( string call ) {
	string token = "Foo";

    try
    {
        auto const host = "slack.com"; //I doubt Slack will change it's URL.
        auto const port = "443";
        string token = SLACK_BOT_TOKEN;
        string target = "/api/rtm." + call + "?token=" + token;
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
        beast::get_lowest_layer(stream).connect(results); //TODO: Find out why this warns in Eclipse
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
	for (rapidjson::Value::ConstValueIterator itr = slack::format["users"].Begin(); itr != slack::format["users"].End(); ++itr) { // Ok
	    if ( itr->HasMember("id") && (*itr)["id"].GetString() == user  ) { // Ok
	    	return (*itr)["profile"]["display_name_normalized"].GetString();
	    }
	}

	return user; //TODO: If we can't find, try making a new request.
}
