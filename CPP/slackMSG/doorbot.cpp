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

bool doorlocked = true;

std::string slack::doorstatus() {
	if (doorlocked) {
		return "locked";
	}
	return "unlocked";
}

void slack::slackDoorbotHandle( string text, string user, string channel, string event_ts ) {
	regex e;
	e = ("Door unlocked by .+ at .+");
	if ( regex_match(text , e) ) {
		doorlocked = false;
	}

	e = ("Door locked by .+ at .+");
	if ( regex_match(text , e) ) {
		doorlocked = true;
	}
}
