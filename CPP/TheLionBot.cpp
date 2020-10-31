/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : TheLionBot.cpp
 Authors     : Alan Percy Childs
 Version     :
*******************************************************************************/

#include "TheLionBot.hpp"

#include <iostream>
#include "Passwords.h" //This needs replacing as an externally read file.

using namespace std;

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = net::ssl;       // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

string slackTocken( ) {
	string token = "Foo";

    try
    {
        auto const host = "slack.com"; //I doubt Slack will change it's URL.
        auto const port = "443";
        string token = SLACK_BOT_TOKEN;
        string target = "/api/rtm.start?token=" + token;
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

    	rapidjson::Document slackJSON;

    	slackJSON.Parse(raw.c_str());

        // Write the message to standard out
//    	cout << slackJSON["url"].GetString() << endl;

    	const auto URL = LUrlParser::ParseURL::parseURL(slackJSON["url"].GetString());

    	if (URL.isValid())
    	{
    		cout << "Scheme    : " << URL.scheme_ << endl;
    		cout << "Host      : " << URL.host_ << endl;
    		cout << "Path      : " << URL.path_ << endl;
    		cout << endl;
    	}
    	else
    	{
    		cout << "URL Parsing error: " << URL.errorCode_ << endl;
    	}

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
    	return URL.path_;
    }
    catch(std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return "Failed"; //TODO: Probably want to handle gracefully.
    }

}


int main(int argc, char** argv)
{
	try
    {
        ssl::context ctx{ssl::context::tlsv12_client};
        load_root_certificates(ctx);

        string host = "cerberus-xxxx.lb.slack-msgs.com";
        auto port = "443";
        string url = "/" + slackTocken( );

        //Hello World!
        //C0U8Y6BQW is the Norwich Hackspace channel ID for #random C0U8Y6BQW
        //D81AQQPFT is the DM for Alan <--> TheLion
        string text = " { \"channel\" : \"D81AQQPFT\" , \"text\" : \"Hello World!\" , \"type\" : \"message\" } ";

        net::io_context ioc;

        tcp::resolver resolver{ioc};
        websocket::stream<beast::ssl_stream<tcp::socket>> ws{ioc, ctx};
        auto const results = resolver.resolve(host, port);
        auto ep = net::connect(get_lowest_layer(ws), results);
        if(! SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host.c_str()))
            throw beast::system_error(
                beast::error_code(
                    static_cast<int>(::ERR_get_error()),
                    net::error::get_ssl_category()),
                "Failed to set SNI Hostname");

        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        host += ':' + std::to_string(ep.port());

        ws.next_layer().handshake(ssl::stream_base::client);
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-coro");
            }));
        ws.handshake(host, url);

        beast::flat_buffer buffer;

        ws.read(buffer); //TODO: Thread this if concurrentcy is needed later on

        // Send the message
        cout << "WRITE: " << text << endl << endl;
        ws.write(net::buffer(text));

        // Read a message into our buffer
        while ( 1 ) {
        	ws.read(buffer); //TODO: Thread this if concurrentcy is needed later on
        	cout << "READ: " << beast::make_printable(buffer.data()) << endl << endl;
        	buffer.clear();
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
        std::cout << beast::make_printable(buffer.data()) << std::endl;
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
