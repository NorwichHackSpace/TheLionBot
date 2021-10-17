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
asio::deadline_timer signoutWait_timer(doorbot_io);
asio::deadline_timer lockedWait_timer(doorbot_io);

std::string slack::doorstatus() {
	if (doorlocked) {
		return "locked";
	}
	return "unlocked";
}

void signinWait( const boost::system::error_code& e ) {
	//Check
	if( e ) { return; }

	//Do
	string responses[] = {
			"Hello? Is someone there?",
			"Ah, a visitor....",
			"Ahwooga, Ahwooga. Unexpected visitor. Ahwooga, Ahwooga!"
	};
	int size = ((&responses)[1] - responses);
	int random = rand() % size;
	slackthread->send(" { \"channel\" : \"" CHAN_WHOISIN "\" , \"text\" : \"" + responses[random] + "\" , \"type\" : \"message\" } ");
}

void signoutWait( const boost::system::error_code& e ) {
	//Check
	if( e ) { return;  } // we were cancelled

	//Do
	string responses[] = {
			"Is anyone still in?\\n",
			"Has everyone left?\\n"
	};
	int size = ((&responses)[1] - responses);
	int random = rand() % size;
	slackthread->send(" { \"channel\" : \"" CHAN_WHOISIN "\" , \"text\" : \"" + responses[random] + "\" , \"type\" : \"message\" } ");
}

void lockedWait( const boost::system::error_code& e ) {
	//Check
	if( e ) { return; }

	//Do
	string responses[] = {
			"Looks like the door is unlocked but nobody is in?\\n",
			"Has everyone left? The door's still unlocked.\\n"
	};
	int size = ((&responses)[1] - responses);
	int random = rand() % size;
	slackthread->send(" { \"channel\" : \"" CHAN_WHOISIN "\" , \"text\" : \"" + responses[random] + "\" , \"type\" : \"message\" } ");
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
			if ( occupiers.Size() ) { //Previous occupiers in this case
				sql = "SELECT * FROM whoisin WHERE timeOut = 0 ORDER BY timeIn DESC LIMIT 0, 1;"; //Create SQL statement
				rapidjson::Document json = database.exec(sql);
				assert(json.IsObject());
				const rapidjson::Value& occupiers = json["db"];
				if ( !occupiers.Size() ) { //The last person just logged out
		   			std::thread doorbot_call{[](){
		   				//int slack_timeout = std::stoi( settings.GetValue("Slack", "IdleTimeout", SLACK_TIMEOUT) );
		   				lockedWait_timer.expires_from_now( boost::posix_time::minutes(10) ); //TODO: Change time allowance
		   				lockedWait_timer.async_wait( lockedWait ); //Recursive
		   				doorbot_io.run();
		   			}};
		   			doorbot_call.detach();
		   			return;
				}
			} else {
				sql = "SELECT * FROM whoisin WHERE timeOut = 0 ORDER BY timeIn DESC LIMIT 0, 10;"; //Someone still in
				rapidjson::Document json = database.exec(sql);
				assert(json.IsObject());
				const rapidjson::Value& occupiers = json["db"];
				if ( occupiers.Size() == 1 ) {
		   			std::thread doorbot_call{[](){
		   				//int slack_timeout = std::stoi( settings.GetValue("Slack", "IdleTimeout", SLACK_TIMEOUT) );
		   				signoutWait_timer.expires_from_now( boost::posix_time::minutes(10) ); //TODO: Change time allowance
		   				signoutWait_timer.async_wait( signoutWait ); //Recursive
		   				doorbot_io.run();
		   			}};
		   			doorbot_call.detach();
					return;
				}
			}

			//Confirmed someone opened the door, but nobody has said they are in or leaving recently. Start the clock!
   			std::thread doorbot_call{[](){
   				//int slack_timeout = std::stoi( settings.GetValue("Slack", "IdleTimeout", SLACK_TIMEOUT) );
   				signinWait_timer.expires_from_now( boost::posix_time::minutes(5) ); //TODO: Change time allowance
   				signinWait_timer.async_wait( signinWait ); //Recursive
   				doorbot_io.run();
   			}};
   			doorbot_call.detach();
   			return;
		}

		e = ("Door( is currently | )locked.*");
		if ( regex_match(text , e) ) {
			doorlocked = true;
			lockedWait_timer.cancel();
		}
}
