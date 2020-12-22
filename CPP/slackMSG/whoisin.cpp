/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : TheLionBot.cpp
 Authors     : Alan Percy Childs
 Version     :

 For designing expressions you might want to try -> https://regexr.com/

*******************************************************************************/

#include "../slack.hpp"
#include <regex>

using namespace std;

string occupying = "";

response leaving;
response arriving;
std::string slack::amendlog( string text, string user ) {

	string response;
	regex e;

	e = (".*([Oo]ut|[Ll]eft|[Ll]eaving).*");
	if ( regex_match(text , e) ) {
		if (occupying == user) occupying = "";
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
		occupying = user;
		string responses[] = {
					"Welcome to the Hackspace " + slack::usertoname(user),
					"Enjoy your stay " + slack::usertoname(user),
					"Thank you for letting us know!",
					"Fire the lasers, pew, pew, pew!",
					"Let the magic commence!"
		};
		int size = ((&responses)[1] - responses);
		response = responses[arriving.random(size)];
	}

 return response;
}

response emptying;
std::string slack::amendlog( unsigned int population ) { //Everybody out!
	string response;

	if (!population) {
		occupying = "";
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
	}

	return response;
}

response emptied;
response ishere;
std::string slack::occupancy() {
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
		response = responses[emptied.random(size)];
	} else {
		string responses[] = {
					"I think " + slack::usertoname(occupying) + " is in.",
					"Looks like " + slack::usertoname(occupying) + " is about.",
					"Looks like " + slack::usertoname(occupying) + " is in.",
					"Looks like " + slack::usertoname(occupying) + " is here.",
					slack::usertoname(occupying) + " is here.",
					slack::usertoname(occupying) + " checked in.",
					slack::usertoname(occupying) + " should be around",
		};
		int size = ((&responses)[1] - responses);
		response = responses[ishere.random(size)];
	}

	return response;
}
