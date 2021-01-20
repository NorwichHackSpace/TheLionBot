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
#include "wiki.hpp"
#include "database.hpp" //SQLite3
#include <boost/bind.hpp>

#include <iostream>
#include <thread>

#include "Passwords.h"

using namespace std;
using namespace slack;
namespace asio = boost::asio;

//------------------------------------------------------------------------------

//Global Uglys
rapidjson::Document slack::startJSON; //Slack info, slack info everywhere!

//Functions for main
void fail(beast::error_code ec, char const* what)
{
    std::cout << what << ": " << ec.message() << std::endl;
}


//Setup shared WS handle
auto const slackthread = boost::make_shared<shared_state>("ws");

//Setup timers for API calling globally to allow recursion
asio::io_context api_io(1);
asio::deadline_timer wiki_timer(api_io, boost::posix_time::seconds(10)); //Time to first call only

string wiki_last_edit_time = "";
void wikitest( const boost::system::error_code& e ) {
	//Check
	if( e ) return; // we were cancelled
	//Do
	rapidjson::Document replyJSON;
	 replyJSON.Parse( wiki::LastEdit().c_str() );
	 rapidjson::Value& wiki = replyJSON["query"]["recentchanges"][0];
	string timestamp = wiki["timestamp"].GetString();
	if (wiki_last_edit_time == "") { //First time running
		wiki_last_edit_time = timestamp;
	} else if (timestamp != wiki_last_edit_time) {
		wiki_last_edit_time = timestamp;
		string type = wiki["type"].GetString();
		string page = wiki["title"].GetString();
		std::replace(page.begin(), page.end(), ' ', '_'); //Need to add underscores for URL
		string response = "Detected " + type + " of page https:\\/\\/wiki.norwichhackspace.org\\/index.php?title=" + page + " at " + timestamp  + ". \\n ";
		//Wiki Channel = CML8QJ3U3 https://wiki.norwichhackspace.org/index.php?title=
		slackthread->send(" { \"channel\" : \"CML8QJ3U3\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } ");
	}
	//Reschedule
	wiki_timer.expires_at(wiki_timer.expires_at() + boost::posix_time::seconds(20));
	wiki_timer.async_wait(wikitest);
}

//And obviously, the main function...
int main(int argc, char** argv)
{
	srand(time(0)); // Make our random a new random.
	try
    {

        // Launch the Slack asynchronous operation
        net::io_context slack_io;
        ssl::context ctx{ssl::context::tlsv12_client};
        load_root_certificates(ctx);
        //auto id = "ws";
        //auto const slackthread = boost::make_shared<shared_state>(id);

        //State flags
       	bool firstrun = true;
       	bool threadsafe = true;

       	while (threadsafe) { //TODO: Make use of threadsafe. So far this will always be true.

       		boost::make_shared<ws_session>(
       			  slack_io
        		, ctx
				, slackthread
				)->do_start();

       		//Slack connection can now be called globally.

       		if (firstrun) {
       	        /*
       	         * Find sending a build message handy for dev work...
       	         * Send Percy a 'version' alert that we have restarted
       	         * C0U8Y6BQW is the Norwich Hackspace channel ID for #random
       	         * CUQV9AGBW is the Norwich Hackspace channel ID for #door-status
       	         * D81AQQPFT is the DM for Alan <--> TheLion
       	         */
       			string buildMSG = " { \"channel\" : \"D81AQQPFT\" , \"text\" : \"Started build " __DATE__ " " __TIME__ "! :lion_face: \" , \"type\" : \"message\" } ";
       			slackthread->send(buildMSG); //Add buildMSG to queue, which the handle will send when ready.
       			firstrun = false;
       		} else {
       			slackthread->send(" { \"channel\" : \"D81AQQPFT\" , \"text\" : \"A new connection established. :lion_face: \" , \"type\" : \"message\" } "); //Just for Debug
       		}

       		slack_io.run_for(std::chrono::seconds(1)); //Block while Slack connection settles

   			//Start the timer operations and split onto another thread, keeping Slack IO responsive.
   			std::thread api_call{[](){
   				wiki_timer.async_wait( wikitest ); //Recursive
   				api_io.run();
   			}};
   			//api_call.join();  //This blocks the Slack thread!

       		while (slack_io.run_one() ) { //Stop if the Slack websocket breaks.
       			/*
       			 * Anything here will effect Slack responsiveness, but if it's quick probably won't get noticed.
       			 * One completion of the loop occurs each time Slack IO does something - most likely a user sent a message.
       			 * It might even be better to just forgo the while() and just call slack_io.run() as a blocking call.
       			 */
       		}

       		api_call.detach();  //Stop other threads becoming zombies. Without this, the program will crash.
       		slack_io.restart(); //Make sure we start a new connection and don't try to reestablish the old one.

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
