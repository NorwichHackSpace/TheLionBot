/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : TheLionBot.cpp
 Authors     : Alan Percy Childs
 Version     :
 Wiki Page	 : https://wiki.norwichhackspace.org/index.php?title=Slack
 Disclaimer  : Any resemblance to actual robots would be really cool
*******************************************************************************/

#include "TheLionBot.hpp"

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

string slackHTTP( string call ) {
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

rapidjson::Document slackStart;

string usertoname( string user) {
	// TODO: Check 'user' is a Slack user ID and not already converted
	// bool string match  -- name begins with 'U' has a number, all letters in caps, and is nine chars long -- , user
	//

	// else if needs converting
	for (rapidjson::Value::ConstValueIterator itr = slackStart["users"].Begin(); itr != slackStart["users"].End(); ++itr) { // Ok
	    if ( itr->HasMember("id") && (*itr)["id"].GetString() == user  ) { // Ok
	    	return (*itr)["profile"]["display_name_normalized"].GetString();
	    }
	}

	return user; //TODO: If we can't find, try making a new request.
}


int main(int argc, char** argv)
{
	try
    {
        ssl::context ctx{ssl::context::tlsv12_client};
        load_root_certificates(ctx);

        /*
         * To establish a Websocket we first need to request a new token from Slack via rtc.start.
         * This is then used to form the URL in a new WSS request, instead of upgrading the connection.
         *
         * As a bonus, we end up with a JSON of user and channel details also.
         *
         */

    	slackStart.Parse(slackHTTP("start").c_str());
    	LUrlParser::ParseURL slackWSurl = LUrlParser::ParseURL::parseURL(slackStart["url"].GetString());

    	if (slackWSurl.isValid())
    	{
    		cout << "Scheme    : " << slackWSurl.scheme_ << endl;
    		cout << "Host      : " << slackWSurl.host_ << endl;
    		cout << "Path      : " << slackWSurl.path_ << endl;
    		cout << endl;
    	}
    	else
    	{
    		cout << "URL Parsing error: " << slackWSurl.errorCode_ << endl;
    	}

        const char * port = "443";
        string path = "/" + slackWSurl.path_;

        // Now we have the required token with the 'path' we can use that to establish WSS

        net::io_context ioc;

        tcp::resolver resolver{ioc};
        websocket::stream<beast::ssl_stream<tcp::socket>> ws{ioc, ctx};
        auto const results = resolver.resolve(slackWSurl.host_, port);
        auto ep = net::connect(get_lowest_layer(ws), results);
        if(! SSL_set_tlsext_host_name(ws.next_layer().native_handle(), slackWSurl.host_.c_str()))
            throw beast::system_error(
                beast::error_code(
                    static_cast<int>(::ERR_get_error()),
                    net::error::get_ssl_category()),
                "Failed to set SNI Hostname");

        slackWSurl.host_ += ':' + std::to_string(ep.port());

        ws.next_layer().handshake(ssl::stream_base::client);
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-coro");
            }));
        ws.handshake(slackWSurl.host_, path);

        beast::flat_buffer buffer;

        ws.read(buffer); //TODO: Thread this if concurrentcy is needed later on

        /*
        //Send the message a 'Hello World!' message
        //C0U8Y6BQW is the Norwich Hackspace channel ID for #random C0U8Y6BQW
        //D81AQQPFT is the DM for Alan <--> TheLion
        string text = " { \"channel\" : \"D81AQQPFT\" , \"text\" : \"Hello World!\" , \"type\" : \"message\" } ";
        cout << "WRITE: " << text << endl << endl;
        ws.write(net::buffer(text));
        */

        // Read a message into our buffer
        while ( ws.is_open() ) {
        	buffer.clear();
        	ws.read(buffer); //TODO: Thread this if concurrentcy is needed later on

        	rapidjson::Document slackRead;
        	string buf = beast::buffers_to_string(buffer.data());
        	slackRead.Parse( buf.c_str() );

        	cout << "READ: " << buf << endl << endl;

        	//If statements read left to right, so second statement is only checked if first does.
        	if ( slackRead.HasMember("type") && !slackRead.HasMember("subtype") && slackRead["type"] == "message" ) { //Simple first message received
        		// Get the strings
        		string channel = slackRead["channel"].GetString();
        		string user = slackRead["user"].GetString();
        		string text = slackRead["text"].GetString();
				string event = slackRead["event_ts"].GetString();
				// Process the strings
        		if (channel == "CUQV9AGBW" && user == "CMFJQ7NNB") { //Only for Dootbot messages in the #door-status channel
        			slackDoorbotHandle( text, user, channel, event );
        		} else {
        			text = slackMsgHandle ( text, user, channel, event ); //Split into message.cpp
        			if (text != "") {
        				cout << "WRITE: " << text << endl << endl;
        				ws.write(net::buffer(text));
        			}
        		}
        	}
        	/* TODO: Handle the server calling timeout, reconnect and avoid with ping messages.
        	 *
        	 * {"type": "goodbye", "source": "gateway_server"}
        	 *
        	 */
        }

        // Close the WebSocket connection
        ws.close(websocket::close_code::normal);

        // If we get here then the connection is closed gracefully

        // The make_printable() function helps print a ConstBufferSequence
        std::cout << beast::make_printable(buffer.data()) << std::endl; //Message why we closed

    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
