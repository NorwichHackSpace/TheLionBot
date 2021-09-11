/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : TheLionBot.cpp
 Authors     : Alan Percy Childs
 Version     :

 For designing expressions you might want to try -> https://regexr.com/

*******************************************************************************/

#include "../slack.hpp"
#include "../database.hpp"
#include <regex>

using namespace std;
#define BUGLINE std::cout << "PASSED LINE " << __LINE__ << " inside " << __FILE__ << std::endl;

/*
 * BEGIN NEW STUFF
 */
template <typename TimePoint> std::string to_string(const TimePoint& time_point) {
    return std::to_string(time_point.time_since_epoch().count());
}
void statPooler() {
		//Convert data from whoisin to the minute stats, the minute stats to the hour, etc... and save into the database.

		//Find first minute to parse from 'whoisin'
		string sql;
		sql = "SELECT timeIn FROM whoisin ORDER BY timeIn ASC LIMIT 0, 1;"; //Create SQL statement
		rapidjson::Document json = database.exec(sql);
		assert(json.IsObject());
		const rapidjson::Value& firstTime = json["db"][0]["timeIn"];
		std::cout << firstTime.GetString() << std::endl;

		//Cast the string-time to a chrono object
		// https://stackoverflow.com/questions/21021388/how-to-parse-a-date-string-into-a-c11-stdchrono-time-point-or-similar
		std::tm tm = {};
		//           2021-05-04 21:27:39     "%Y-%m-%e %H:%M:%S"
		//strptime("Thu Jan 9 2014 12:35:34", "%a %b %d %Y %H:%M:%S", &tm);
		strptime(firstTime.GetString(), "%Y-%m-%e %H:%M:%S", &tm);
		auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

		//Cast the chrono object to time points
	    using namespace std::literals::chrono_literals;
	    using Sec = std::chrono::seconds;
	    using Hour = std::chrono::hours;

	    std::cout << "Time In\t\t" "Cast\t\n";
	    std::cout << "(ms)\t\t"  "(s)\t\t" "(hr)\t\t \n";
        std::cout
            << to_string(tp) << "\t\t"
            << to_string(std::chrono::time_point_cast<Sec>(tp)) << "\t\t"
            << to_string(std::chrono::time_point_cast<Hour>(tp)) << "\t\n";

}

response leaving;
response arriving;

std::string slack::amendlog( string text, string user ) {

	string sql;
	string response;
	regex e;

	e = (".*([Oo]ut|[Ll]eft|[Ll]eaving).*");
	if ( regex_match(text , e) ) {
		sql = "UPDATE whoisin SET timeOut=datetime('now') WHERE id='" + user + "' AND timeOut=0;"; //Create SQL statement
		string responses[] = {
					"Hope it went well " + slack::usertoname(user) + "!",
					"Please make sure the door is closed properly, it's a bit sticky sometimes.",
					"See ya later " + slack::usertoname(user) + "!",
					"See you soon " + slack::usertoname(user) + "!",
					"Laters!",
					"Don't forget the lights!",
					"It's been nice company. :-)",
					"Till next time!",
		};
		int size = ((&responses)[1] - responses);
		response = responses[leaving.random(size)];
	} else { // If not leaving, must be coming in...
		sql = "INSERT INTO whoisin(timeIn,id,timeOut) VALUES ( datetime('now') ,'" + user + "',0);"; //Create SQL statement
		string responses[] = {
					"Welcome to the Hackspace " + slack::usertoname(user),
					"Enjoy your stay " + slack::usertoname(user),
					"Thank you for letting us know!",
					"Fire the lasers, pew, pew, pew!",
					"Let the magic commence!"
		};
		int size = ((&responses)[1] - responses);
		response = responses[arriving.random(size)];
		statPooler();
	}
 database.exec(sql);
 return response;
}

response emptying;
std::string slack::amendlog( unsigned int population ) { //Everybody out!
	string sql;
	string response;

	if (!population) {
		sql = "UPDATE whoisin SET timeOut=datetime('now') WHERE timeOut=0;"; //Create SQL statement
		string responses[] = {
				"Thanks for the update!",
				"Brill, I'll make a note of it.",
				"Well, everyone but me. I'll keep an eye. O.o ",
				"Sleepy time.",
				"Hope everyone had fun.",
				"What a party, huh?",
				"It was fun!",
				"Even the cat!",
				"Phew!",
				"If no one is in to see the place empty, how do you know it is empty? My logic circuits are frazzled.",
				"Thanks for checking.",
  	};
		int size = ((&responses)[1] - responses);
		response = responses[emptying.random(size)];
		if ( slack::doorstatus() == "unlocked" ) {
			response += "\\nCan you make sure to lock the door please?";
		}
	}
	database.exec(sql);
	return response;
}

response emptied;
response ishere;
std::string slack::occupancy() {
	string response;
	string sql;

	sql = "SELECT * FROM whoisin WHERE timeOut=0 ORDER BY timeIn ASC LIMIT 0, 10;"; //Create SQL statement
	rapidjson::Document json = database.exec(sql);
	assert(json.IsObject());
	const rapidjson::Value& occupiers = json["db"];

	if ( !occupiers.Size() ) {
		string responses[] = {
					"Looks empty.",
					"Don't think anyone is in.",
					"Nobody as far as I know.",
					"Lights are out, nobody home.", //TODO: We should probably check the lights really are out.
					"Not aware of anyone being in.",
					"Users are NaS. That's Not At Space.",
					"Did someone come in without me seeing?",
		};
		int size = ((&responses)[1] - responses);
		response = responses[emptied.random(size)];
	} else {
		assert(occupiers.IsArray());
		response = "";
		string occupying = "";
		rapidjson::Value::ConstValueIterator itr;
		for (itr = occupiers.Begin(); itr != occupiers.End(); ++itr ) {
			if ( !(itr == occupiers.Begin()) ) {
				if (occupiers.Size() > 2) { occupying.append(", "); }
				if ( ++itr == occupiers.End() ) {
					if (occupiers.Size() < 3) { occupying.append(" "); }
					occupying.append("and ");
				}
				itr--;
			}
			occupying.append(
					slack::usertoname( (*itr)["id"].GetString() )
			);
		}
			if (occupiers.Size() == 1) {
				occupying.append(" is");
			} else {
				occupying.append(" are");
			}
			string responses[] = {
						"I think " + occupying + " in. ",
						"Looks like " + occupying + " about. ",
						"Looks like " + occupying + " in. ",
						"Looks like " + occupying + " here. ",
						occupying + " here. ",
						occupying + " in. ",
						occupying + " around. ",
			};
			int size = ((&responses)[1] - responses);
			response.append(responses[ishere.random(size)]);
	}

	return response;
}
