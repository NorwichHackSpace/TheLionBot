/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : TheLionBot.cpp
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

rapidjson::Document slack::format; //Slack info, slack info everywhere!

void
fail(beast::error_code ec, char const* what)
{
    std::cout << what << ": " << ec.message() << std::endl;
}

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
        ssl::context ctx{ssl::context::tlsv12_client};
        load_root_certificates(ctx);

        // Launch the asynchronous operation
        auto id = "ws";
        auto const handle = boost::make_shared<shared_state>(id);
        boost::make_shared<ws_session>(
        		  ioc
				, ctx
				, handle
				)->do_start(
        		slackWSurl.host_
				, port
				, path
		);

        /*
         * Find sending a build message handy for dev work...
         * Send Percy and #door-status a 'version' alert that we have restarted
         * C0U8Y6BQW is the Norwich Hackspace channel ID for #random
         * CUQV9AGBW is the Norwich Hackspace channel ID for #door-status
         * D81AQQPFT is the DM for Alan <--> TheLion
         */
        string buildMSG = " { \"channel\" : \"D81AQQPFT\" , \"text\" : \"Started build " __DATE__ " " __TIME__ "! :lion_face: \" , \"type\" : \"message\" } ";
		handle->send(buildMSG); //Add buildMSG to queue, which handle will send when ready.

        //We could do all sorts here, while the Slack stuff is running async, in the background...
       	std::cout << "Websocket doing stuff while I send this. Threading magic behold!" << endl;

/*
       	while (1) {
       		sleep(120);
       		do something fancy
       	}
*/

       	ioc.run(); //Block until the websockets are closed
        std::cout << "ASync Finished!" << endl;

    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
