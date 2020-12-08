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

string slackMsgHandle( string text, string user, string channel, string event_ts ) {
	string JSON = "";
	regex e;

//who am I?
	e = ("[wW]ho(\\s|m\\s)am [Ii][?.\\n]");
	if ( regex_match(text , e) ) {
		string response = whoami(user);
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
	}


//who is in?
	//static string usersPresent[];
	//Door-status channel == CUQV9AGBW/
	e = ("[wW]ho(m|)(s| is)(.currently.|\\W)(in|at|around|present)(\\s|[a-z])*[\\.?\n]");
	if ( regex_match(text , e) ) {
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + "Don't ask me!" + "\" , \"type\" : \"message\" } " ;
	}

	e = ("([Ww]hat.|)([Ii]s) the( | front | main )(door|(.|)space).(status|open|unlocked|locked|closed|shut)(|.)");
	if ( regex_match(text , e) ) {
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + "The front door is currently " + doorstatus() +  "\" , \"type\" : \"message\" } " ;
	}


return JSON;
}
