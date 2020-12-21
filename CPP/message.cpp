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

//random debugging thingy
	//Based on the repeating responses, it appears the rand() function is very unrandom the way I'm using it.
	//Adding this just while I test just how truly unrandom it is, by seeing how many times certain numbers repeat.
	e = ("(Give us some random, Lion.)"); //Be specific for now, this is just for debuggin.
	#define NUM_RANDOMS 100000
	#define HIGHEST_RANDOM 10
	if ( regex_match(text , e) ) {
		string response = "I just generated " + to_string(NUM_RANDOMS) + " Randoms between 0 and " + to_string(HIGHEST_RANDOM) + ":  \\n";
		string rndams_string = "";
		unsigned int rndams[NUM_RANDOMS] = {};
		for (unsigned int i = 0; i < NUM_RANDOMS; i++) {
			rndams[i] = (rand() % HIGHEST_RANDOM); //Pick a 'random' number between 0 and 100.
		}
		unsigned int nums_of_value;
		for (unsigned int i = 0; i < HIGHEST_RANDOM; i++) { //Number to match
			nums_of_value = 0;
			for (unsigned int elems = 0; elems < NUM_RANDOMS; elems++) {
				if (rndams[elems] == i) { nums_of_value++; } //Check if the number inside the element matches the number we are looking for.
			}
			response = response + to_string(i) + " occurred " + to_string(nums_of_value) + " times. \\n ";
		}
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
		return JSON;
	}

//jokes
	e = ("([Aa]nother|[Cc]an|[Tt]ell|[Ss]ay|[Gg]ive).*([Jj]oke|[Ff]unny)(| please)(| [Ll]ion)\\W*");
	if ( regex_match(text , e) ) {
		string response = xmas_joke();
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
		return JSON;
	}


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
		if ( !(regex_match(text , e)) ) { //Check this is a command NOT a question handled below
			e = ("^([\\w]*)");
			smatch match;
			regex_search(text, match, e);
			user = match[0] ;
			string response = amendlog( text, user );
			JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
			return JSON;

		}
	}

	e = ("([Ii]n)|([Oo]ut)|(.*([Ii].|)(’m|'m|am|are|[Nn]ow|[Jj]ust) (currently |now |just |got |)(in|arriving|out|here|left|leaving)\\W*)");
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
	e = ("[wW]ho(m|'|’|)(s| is)(.currently.|\\W)(in|at|around|present)( .*|\\W|)");
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

string idle_responses[] = {
		"In the hackspace, the Norwich hackspace \n The lion sleeps tonight \n Wee heeheehee weeoh aweem away \n Wee heeheehee weeoh aweem away \n",
		"RRRRWWWWAAAA. I'm sleepy. Can't you guys do something to keep me awake?"
		"Think I just saw a tumble weed."
};
