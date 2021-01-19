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
#include "database.hpp" //SQLite3

#include <iostream>
#include "Passwords.h"

using namespace std;
using namespace slack;

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = net::ssl;       // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

rapidjson::Document slack::startJSON; //Slack info, slack info everywhere!

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

        // Launch the asynchronous operation
        auto id = "ws";
        net::io_context ioc;
        ssl::context ctx{ssl::context::tlsv12_client};
        load_root_certificates(ctx);
        auto const handle = boost::make_shared<shared_state>(id);


       	bool firstrun = true;
       	while (1) {

       		boost::make_shared<ws_session>(
        		  ioc
        		, ctx
				, handle
				)->do_start();

       		if (firstrun) {
       	        /*
       	         * Find sending a build message handy for dev work...
       	         * Send Percy a 'version' alert that we have restarted
       	         * C0U8Y6BQW is the Norwich Hackspace channel ID for #random
       	         * CUQV9AGBW is the Norwich Hackspace channel ID for #door-status
       	         * D81AQQPFT is the DM for Alan <--> TheLion
       	         */
       			string buildMSG = " { \"channel\" : \"D81AQQPFT\" , \"text\" : \"Started build " __DATE__ " " __TIME__ "! :lion_face: \" , \"type\" : \"message\" } ";
       			handle->send(buildMSG); //Add buildMSG to queue, which handle will send when ready.
       	       	std::cout << "Websocket doing stuff while I cout this. Threading magic behold!" << endl << endl;
       			firstrun = false;
       		} else {
       			handle->send(" { \"channel\" : \"D81AQQPFT\" , \"text\" : \"A new connection established. :lion_face: \" , \"type\" : \"message\" } "); //Just for Debug
       		}

            //We could do all sorts here, while the Slack stuff is running async, in the background...

       		//ioc.run_for(rel_time);
       		while (1) {
       			if (!ioc.run_one() ) { break; } //Stop on errors, instead of looping though them.
       		}

       		//ioc.run(); //Block until the websockets are closed
       		ioc.restart();
       	}

        std::cout << "ASync Finished!" << endl;

    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
