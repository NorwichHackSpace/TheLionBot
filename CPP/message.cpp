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
	//Log
	e = ("([Ii]n)|([Oo]ut)|(.*([Ii].|)('m|am|are|[Nn]ow|[Jj]ust) (currently |now |just |)(in|out|here|left|leaving).*)");
	if ( regex_match(text , e) ) {
		string response = amendlog( text, user );
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
	}
	//TODO: Everyone out!

	//Query
	e = ("[wW]ho(m|'|)(s| is)(.currently.|\\W)(in|at|around|present)(\\s|[a-z])*[\\.?\n]");
	if ( regex_match(text , e) ) {
		string response = occupancy();
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
	}



//is door locked?
	e = ("([Ww]hat.|)([Ii]s) the( | front | main )(door|(.|)space).(status|open|unlocked|locked|closed|shut)(|.)");
	if ( regex_match(text , e) ) {
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + "The front door is currently " + doorstatus() +  "\" , \"type\" : \"message\" } " ;
	}


return JSON;
}
