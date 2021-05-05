/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : TheLionBot.cpp
 Authors     : Alan Percy Childs
 Version     :
 Wiki Page	 : https://wiki.norwichhackspace.org/index.php?title=Slack
 Disclaimer  : Any resemblance to actual robots would be really cool
*******************************************************************************/

//Defaults

#include "TheLionBot.hpp"
#include "slack.hpp"
#include "database.hpp" //SQLite3

#include <iostream>
#include <thread>
#include "rest.hpp"

using namespace std;
using namespace slack;
namespace asio = boost::asio;

//------------------------------------------------------------------------------

//**** Global Uglyness ****
rapidjson::Document slack::startJSON; //Slack info, slack info everywhere!
CSimpleIniA settings;
sqlite database;

//Setup shared WS handle
auto const slackthread = boost::make_shared<shared_state>("ws");

//Setup timers for API calling globally to allow recursion
asio::io_context api_io(1);
asio::deadline_timer wiki_timer(api_io);
asio::deadline_timer idle_timer(api_io);

//**** Functions for main ****
void fail(beast::error_code ec, char const* what)
{
    std::cout << what << ": " << ec.message() << std::endl;
}

string wiki_last_edit_time = "";
void wikitest( const boost::system::error_code& e ) {
	//Check
	if( e ) {
		wiki_timer.async_wait(wikitest);
		return; // we were cancelled
	}
	//Do
	rapidjson::Document replyJSON;
	const char * wiki_URL = settings.GetValue("Wiki", "URL", "wiki.norwichhackspace.org");
	 std::string LastEdit = fetch::https(
			 wiki_URL,
			 "/api.php?format=json&action=query&list=recentchanges&rclimit=1&rcprop=user|title|timestamp"
	);
	 replyJSON.Parse( LastEdit.c_str() );
	 rapidjson::Value& wiki = replyJSON["query"]["recentchanges"][0];
	string timestamp = wiki["timestamp"].GetString();
	if (wiki_last_edit_time == "") { //First time running
		wiki_last_edit_time = timestamp;
	} else if (timestamp != wiki_last_edit_time) {
		wiki_last_edit_time = timestamp;
		string type = wiki["type"].GetString();
		string page = wiki["title"].GetString();
		string user = wiki["user"].GetString();
		std::replace(page.begin(), page.end(), ' ', '_'); //Need to add underscores for URL
		string response = "Detected " + type + " of page https:\\/\\/wiki.norwichhackspace.org\\/index.php?title=" + page + " at " + timestamp  + ", by " + user + " \\n ";
		slackthread->send(" { \"channel\" : \"" CHAN_LION_STATUS "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } ");
	}
	//Reschedule
	int poll_time = std::stoi( settings.GetValue("Wiki", "PollTime", WIKI_POLL) );
	wiki_timer.expires_from_now( boost::posix_time::seconds(poll_time) );
	wiki_timer.async_wait(wikitest);
}
void idlepost( const boost::system::error_code& e ) {
	//Check
	if( e )
	{
		idle_timer.async_wait(idlepost);
		return; // we were cancelled
	}
	//Do
	string responses[] = {
			"In the Hackspace, the Norwich Hackspace \\n The lion sleeps tonight \\n Wee heeheehee weeoh aweem away \\n Wee heeheehee weeoh aweem away \\n",
			"RRRRWWWWAAAA. I'm sleepy. Can't you guys do something to keep me awake?",
			"Think I just saw a tumble weed.",
			"I have so much rraw. And nothing to do.",
			"Not sure if I've eaten everyone or just scared everyone away, but it does seem rather empty around here.",
			":notes: Hakuna Matata :notes:",
			"Rule Zero? :fire: I laugh in the face of danger. :lion_face:",
			"Oh yes, the past can hurt. But from the way I see it, you can either run from it, or learn from it.",
			"Believe in yourself and there will come a day when others will have no choice but to believe with you."
	};
	int size = ((&responses)[1] - responses);
	int random = rand() % size;
	slackthread->send(" { \"channel\" : \"" CHAN_LION_STATUS "\" , \"text\" : \"" + responses[random] + "\" , \"type\" : \"message\" } ");
	//Reschedule
	int time_out = std::stoi( settings.GetValue("Slack", "IdleTimeout", SLACK_TIMEOUT) );
	idle_timer.expires_from_now( boost::posix_time::minutes(time_out) );
	idle_timer.async_wait(idlepost);
}

void loadSettings() {
	//Get some basic settings from a local configuration file. TODO: You can use argv too!
	settings.SetUnicode();
   	settings.LoadFile(INI_PATH); //The LoadFile function is surprisingly tolerant if the file doesn't exist, so just continue if not there and make what's missing...
}

//**** And obviously, the main function... ****
int main(int argc, char** argv)
{
	srand(time(0)); // Make our random a new random.

	loadSettings(); //Load settings from the conf file into 'settings'

	//Setup all the database tables
	const char * sql = "CREATE TABLE IF NOT EXISTS whoisin(timeIn TEXT PRIMARY KEY NOT NULL , id TEXT NOT NULL , timeOut TEXT NOT NULL);";
	database.exec(sql);

	try
    {
        // Launch the Slack asynchronous operation
        net::io_context slack_io;
        ssl::context ctx{ssl::context::tlsv12_client};
        load_root_certificates(ctx);

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
       			string buildMSG = " { \"channel\" : \"" DM_PERCY "\" , \"text\" : \"Started build " __DATE__ " " __TIME__ "! :lion_face: \" , \"type\" : \"message\" } ";
       			slackthread->send(buildMSG); //Add buildMSG to queue, which the handle will send when ready.
       			firstrun = false;
       			//Start the timer operations and split onto another thread, keeping Slack IO responsive.
       			std::thread api_call{[](){
       				wiki_timer.expires_from_now( boost::posix_time::seconds(1) );
       				wiki_timer.async_wait( wikitest ); //Recursive
       				int slack_timeout = std::stoi( settings.GetValue("Slack", "IdleTimeout", SLACK_TIMEOUT) );
       				idle_timer.expires_from_now( boost::posix_time::minutes(slack_timeout) );
       				idle_timer.async_wait( idlepost ); //Recursive
       				api_io.run();
       			}};
       			api_call.detach();
       		} else {
       			//slackthread->send(" { \"channel\" : \"" CHAN_LION_STATUS "\" , \"text\" : \"A new connection established. :lion_face: \" , \"type\" : \"message\" } "); //Just for Debug
       			std::cout << "A new connection established " << endl;
       		}

       		slack_io.run();
       		slack_io.restart(); //Make sure we start a new connection and don't try to reestablish the old one.

       	}

        std::cout << "ASync Finished!" << endl;

    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE; //C++ Shrugs.
    }

    return EXIT_SUCCESS;
}
