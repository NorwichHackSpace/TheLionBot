/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : TheLionBot.cpp
 Authors     : Alan Percy Childs
 Version     :

 For designing expressions you might want to try -> https://regexr.com/

*******************************************************************************/

#include "TheLionBot.hpp"
#include <regex>

using namespace std;

string occupying = "";

std::string amendlog( string text, string user ) {
	string response;
	regex e;

	e = (".*(in|here|arrived).*");
	if ( regex_match(text , e) ) { //In
		occupying = user;
		string responses[] = {
					"Welcome to the Hackspace " + usertoname(user),
					"Enjoy your stay " + usertoname(user),
					"Thank you for letting us know!",
					"Fire the lasers, pew, pew, pew!",
					"Let the magic commence!"
		};
		int size = ((&responses)[1] - responses);
		int random = rand() % size;
		response = responses[random];
	}

	e = (".*(out|left|leaving).*");
	if ( regex_match(text , e) ) { //Out
		occupying = "";
		string responses[] = {
					"Hope it went well " + usertoname(user) + "!",
					"Please make sure the door is closed properly, it's a bit sticky sometimes.",
					"See ya later " + usertoname(user) + "!",
					"See you soon " + usertoname(user) + "!",
					"Laters!",
					"Don't forget the lights!",
					"It's been nice company. :-)",
					"Till next time!",
		};
		int size = ((&responses)[1] - responses);
		int random = rand() % size;
		response = responses[random];
	}

 return response;
}

void occupancy( unsigned int population ) { //Everybody out!
	if (!population) occupying = "";
}

std::string occupancy() {
	string response;

	if (occupying == "") {
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
		int random = rand() % size;
		response = responses[random];
	} else {
		string responses[] = {
					"I think " + usertoname(occupying) + " is in.",
					"Looks like " + usertoname(occupying) + " is about.",
					"Looks like " + usertoname(occupying) + " is in.",
					"Looks like " + usertoname(occupying) + " is here.",
					usertoname(occupying) + " is here.",
					usertoname(occupying) + " checked in.",
					usertoname(occupying) + " should be around",
		};
		int size = ((&responses)[1] - responses);
		int random = rand() % size;
		response = responses[random];
	}

	return response;
}
