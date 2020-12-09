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
		return JSON;
	}


//who is in?
	//Log
	e = ("(([Ii]n).*([Oo]ut))");
	//TODO: When we have a quick visit this is normally used, but not sure how ( or even if ) to handle it yet.

	e = ("(\\w*)( is )(here|in)(.|)");
	if ( regex_match(text , e) ) {

		e = ("([Nn]o)( one|body)(\\s).*");
		if ( (regex_match(text , e)) ) {
			string response = amendlog( 0 );
			JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
			return JSON;
		}

		e = ("([Ww]ho)(m|)(\\s).*");
		if ( !(regex_match(text , e)) ) { //Check this is a command not a question...
			e = ("^([\\w]*)");
			smatch match;
			regex_search(text, match, e);
			user = match[0] ;
			string response = amendlog( text, user );
			JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
			return JSON;

		}
	}

	e = ("([Ii]n)|([Oo]ut)|(.*([Ii].|)(’m|'m|am|are|[Nn]ow|[Jj]ust) (currently |now |just |)(in|out|here|left|leaving).*)");
	if ( regex_match(text , e) ) {
		string response = amendlog( text, user );
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
		return JSON;
	}

	e = ("([Ee]veryone|[Aa]ll).*( out| empty| left).*");
	if ( regex_match(text , e) ) {
			string response = amendlog( 0 );
			JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
			return JSON;
		}

	//Query
	e = ("[wW]ho(m|'|’|)(s| is)(.currently.|\\W)(in|at|around|present)(\\s|[a-z])*[\\.?\n]");
	if ( regex_match(text , e) ) {
		string response = occupancy();
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
		return JSON;
	}



//is door locked?
	e = ("([Ww]hat.|)([Ii]s) the( | front | main )(door|(.|)space).(status|open|unlocked|locked|closed|shut)(|.)");
	if ( regex_match(text , e) ) {
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + "The front door is currently " + doorstatus() +  "\" , \"type\" : \"message\" } " ;
		return JSON;
	}


return JSON;
}
