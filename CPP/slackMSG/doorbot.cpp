/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : TheLionBot.cpp
 Authors     : Alan Percy Childs
 Version     :

 For designing expressions you might want to try -> https://regexr.com/

*******************************************************************************/

#include "../TheLionBot.hpp"
#include "../slack.hpp"
#include "../database.hpp"
#include <regex>
#include <thread>

using namespace std;
namespace asio = boost::asio;

bool doorlocked = true; //TODO: Get from database (and save status to, etc)
asio::io_context doorbot_io(1);
asio::deadline_timer signinWait_timer(doorbot_io);

std::string slack::doorstatus() {
	if (doorlocked) {
		return "locked";
	}
	return "unlocked";
}

void signinWait( const boost::system::error_code& e ) {
	//Check
	if( e )
	{
		return; // we were cancelled
	}

	//Do
	string responses[] = {
			"TEST MSG - Unknown user entered.\\n",
			"TEST MSG - Waiting to find out who just came in.\\n",
	};
	int size = ((&responses)[1] - responses);
	int random = rand() % size;
	slackthread->send(" { \"channel\" : \"" CHAN_LION_STATUS "\" , \"text\" : \"" + responses[random] + "\" , \"type\" : \"message\" } ");

}

void slack::slackDoorbotHandle( string text, string user, string channel, string event_ts ) {
		regex e;
		e = ("Door( is currently | )unlocked.*");
		if ( regex_match(text , e) ) {
			doorlocked = false;

			//Check DB if we have a user update recently
			string sql;

			sql = "SELECT * FROM whoisin WHERE timeOut >= Datetime('now', '-5 minutes') ORDER BY timeIn DESC LIMIT 0, 1;"; //Create SQL statement
			rapidjson::Document json = database.exec(sql);
			assert(json.IsObject());
			const rapidjson::Value& occupiers = json["db"];
			if ( occupiers.Size() ) {
				slackthread->send(" { \"channel\" : \"" CHAN_LION_STATUS "\" , \"text\" : \" DEBUG - DOOR UNLOCKED BUT SOMEBODY SIGNED OUT WITHIN LAST 5 MINUTES\" , \"type\" : \"message\" } ");
				return; //No need to call the timer as it was a safe unlock
			} else {
				sql = "SELECT * FROM whoisin WHERE timeOut = 0 ORDER BY timeIn DESC LIMIT 0, 1;"; //Create SQL statement
				rapidjson::Document json = database.exec(sql);
				assert(json.IsObject());
				const rapidjson::Value& occupiers = json["db"];
				if ( occupiers.Size() ) {
					slackthread->send(" { \"channel\" : \"" CHAN_LION_STATUS "\" , \"text\" : \" DEBUG - DOOR UNLOCKED BUT SOMEBODY ALREADY IN\" , \"type\" : \"message\" } ");
					return; //No need to call the timer as it was a safe unlock
				}
			}

			//Confirmed someone opened the door, but nobody has said they are in or leaving yet. Start the clock!
			slackthread->send(" { \"channel\" : \"" CHAN_LION_STATUS "\" , \"text\" : \" DEBUG - CALLING THE TIMED THREAD \" , \"type\" : \"message\" } ");
   			std::thread doorbot_call{[](){
   				//int slack_timeout = std::stoi( settings.GetValue("Slack", "IdleTimeout", SLACK_TIMEOUT) );
   				signinWait_timer.expires_from_now( boost::posix_time::minutes(1) ); //TODO: Change time allowance
   				signinWait_timer.async_wait( signinWait ); //Recursive
   				doorbot_io.run();
   			}};
   			doorbot_call.detach();

		}

		e = ("Door( is currently | )locked.*");
		if ( regex_match(text , e) ) {
			doorlocked = true;
		}
}
