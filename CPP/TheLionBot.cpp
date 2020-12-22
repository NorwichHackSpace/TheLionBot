/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : TheLionBot.cpp
 Authors     : Alan Percy Childs
 Version     : Test
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

rapidjson::Document slack::format;

int main(int argc, char** argv)
{
	srand(time(0)); // Make our random a new random.
	try
    {

        /*
         * To establish a Websocket we first need to request a new token from Slack via rtc.start.
         * This is then used to form the URL in a new WSS request, instead of upgrading the connection.
         * Note that we could just use rtc.connect but by using rtc.start, as a bonus, we end up with a JSON
         * of user and channel details also.
         */

        slack::format.Parse(slack::HTTP("start").c_str()); //Populate 'format' with data from s Slack Web API call 'start'
    	LUrlParser::ParseURL slackWSurl = LUrlParser::ParseURL::parseURL(slack::format["url"].GetString());

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

        ssl::context ctx{ssl::context::tlsv12_client};
        load_root_certificates(ctx);

        websocket::stream<beast::ssl_stream<tcp::socket>> ws{ioc, ctx};
        auto const results = resolver.resolve(slackWSurl.host_, port);
        auto ep = net::connect(get_lowest_layer(ws), results);
        if(! SSL_set_tlsext_host_name(ws.next_layer().native_handle(), slackWSurl.host_.c_str()))
            throw beast::system_error(
                beast::error_code(
                    static_cast<int>(::ERR_get_error()),
                    net::error::get_ssl_category()),
                "Failed to set SNI Hostname");

        slackWSurl.host_ += ':' + to_string(ep.port());

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

        ws.read(buffer);






        /*
        //Send Percy and #door-status a 'version' alert that we have restarted
        //C0U8Y6BQW is the Norwich Hackspace channel ID for #random
        //CUQV9AGBW is the Norwich Hackspace channel ID for #door-status
        //D81AQQPFT is the DM for Alan <--> TheLion
        */
        string text = " { \"channel\" : \"D81AQQPFT\" , \"text\" : \"Started build " __DATE__ " " __TIME__ "! :lion_face: \" , \"type\" : \"message\" } ";
        cout << "WRITE: " << text << endl << endl;
        ws.write(net::buffer(text));
        //text = " { \"channel\" : \"CUQV9AGBW\" , \"text\" : \"Started build " __DATE__ " " __TIME__ "! :lion_face: \" , \"type\" : \"message\" } ";
        //ws.write(net::buffer(text));

        // Read a message into our buffer
        while ( ws.is_open() ) {
        	buffer.clear();
        	srand(time(0)); // Normally we wouldn't need to recall a srand()
        	ws.read(buffer); //TODO: Thread this if concurrency is needed later on

        	rapidjson::Document slackRead;
        	string buf = beast::buffers_to_string(buffer.data());
        	slackRead.Parse( buf.c_str() );

        	cout << "READ: " << buf << endl << endl;

        	if ( slackRead.HasMember("type") && !slackRead.HasMember("subtype") && slackRead["type"] == "message" ) { //Simple first message received
        		// Get the strings
        		string channel = slackRead["channel"].GetString();
        		string user = slackRead["user"].GetString();
        		string text = slackRead["text"].GetString();
				string event = slackRead["event_ts"].GetString();
				// Process the strings
        		if (channel == "CUQV9AGBW" && user == "CMFJQ7NNB") { //Only for Dootbot messages in the #door-status channel
        			slack::slackDoorbotHandle( text, user, channel, event );
        		} else {
        			text = slack::slackMsgHandle ( text, user, channel, event ); //Split into message.cpp
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
