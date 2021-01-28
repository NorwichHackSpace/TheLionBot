/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : TheLionBot.cpp
 Authors     : Alan Percy Childs
 Version     :

 For designing expressions you might want to try -> https://regexr.com/

*******************************************************************************/

#include "../TheLionBot.hpp"
#include "../slack.hpp"
#include "../wiki.hpp"
#include <regex>

using namespace std;

string slack::slackMsgHandle( string text, string user, string channel, string event ) {

	string JSON = "";
	regex e;

//Wiki test
	e = ("(lion:wiki)"); //Be specific for now, this is just for debuggin.
	if ( regex_match(text , e) ) {
		rapidjson::Document replyJSONa;
		 replyJSONa.Parse( wiki::LastEdit().c_str() );
		 rapidjson::Value& wiki = replyJSONa["query"]["recentchanges"][0];
		string type = wiki["type"].GetString();
		string page = wiki["title"].GetString();
		string timestamp = wiki["timestamp"].GetString();
		string response = "Last Wiki " + type + " was on page " + page + " at " + timestamp  + ". \\n ";
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
		return JSON;
	}

//Celebrate an update!
	e = ("(lion:updatecelebrate)"); //Be specific for now, this is just for showing off.
	if ( regex_match(text , e) ) {
		string response = "Rarrrrr! I was freshly groomed " __DATE__ " " __TIME__ "! :lion_face: \\n ";
		channel = "C0U8Y6ALE"; //Post to #general, regardless of where called
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
		return JSON;
	}

//random debugging thingy
	//Based on the repeating responses, it appears the rand() function is very unrandom the way I'm using it.
	//Adding this just while I test just how truly unrandom it is, by seeing how many times certain numbers repeat.
	e = ("(lion:randomtest)"); //Be specific for now, this is just for debuggin.
	#define NUM_RANDOMS 1000000
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
		string response = slack::joke();
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
		return JSON;
	}


//who am I?
	e = ("[wW]ho(\\s|m\\s)am [Ii][?.\\n]");
	if ( regex_match(text , e) ) {
		string response = slack::whoami(user);
		//JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
		//Made a threaded response
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" , \"thread_ts\" : \"" + event + "\" } " ;
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
			string response = slack::amendlog( 0 );
			JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
			return JSON;
		}

		e = ("([Ww]ho)(m|)(\\s).*");
		if ( !(regex_match(text , e)) ) { //Check this is a command NOT a question handled below
			e = ("^([\\w]*)");
			smatch match;
			regex_search(text, match, e);
			user = match[0] ;
			string response = slack::amendlog( text, user );
			JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
			return JSON;

		}
	}

	e = ("([Ii]n)|([Oo]ut)|(.*([Ii].|)(’m|'m|am|are|[Nn]ow|[Jj]ust) (currently |now |just |got |)(in|arriving|out|here|left|leaving)\\W*)");
	if ( regex_match(text , e) ) {
		string response = slack::amendlog( text, user );
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
		return JSON;
	}

	e = ("([Ee]veryone|[Aa]ll).*( out| empty| left).*");
	if ( regex_match(text , e) ) {
			string response = slack::amendlog( 0 );
			JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
			return JSON;
		}

	//Query
	e = ("[wW]ho(m|'|’|)(s| is)(.currently.|\\W)(in|at|around|present)( .*|\\W|)");
	if ( regex_match(text , e) ) {
		string response = slack::occupancy();
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
		return JSON;
	}



//is door locked?
	e = ("([Ww]hat.|)([Ii]s) the( | front | main )(door|(.|)space).(status|open|unlocked|locked|closed|shut)(|.)");
	if ( regex_match(text , e) ) {
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + "The front door is currently " + slack::doorstatus() +  "\" , \"type\" : \"message\" } " ;
		return JSON;
	}


return JSON;
}


