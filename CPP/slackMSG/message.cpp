/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : TheLionBot.cpp
 Authors     : Alan Percy Childs
 Version     :

 For designing expressions you might want to try -> https://regexr.com/

*******************************************************************************/

#include "../TheLionBot.hpp"
#include "../slack.hpp"
#include "../rest.hpp"
#include "../database.hpp"
#include <regex>

using namespace std;
response useemoji;
string slack::slackMsgHandle( string text, string user, string channel, string event ) {

	string JSON = "";
	regex e;

//Doorbot
	if ( user == USER_DOORBOT || ( channel == CHAN_DOORSTATUS &&  user == USER_PERCY) ) {
		slack::slackDoorbotHandle (text, user, channel, event);
		JSON = "";
		return JSON;
	}

//Wiki test
	e = ("(lion:wiki)"); //Be specific for now, this is just for debuggin.
	if ( regex_match(text , e) ) {
		rapidjson::Document replyJSONa;
		const char * wiki_URL = settings.GetValue("Wiki", "URL", "wiki.norwichhackspace.org");
		 std::string LastEdit = fetch::https(
				 wiki_URL,
				 "/api.php?format=json&action=query&list=recentchanges&rclimit=1&rcprop=user|title|timestamp"
		);
		 replyJSONa.Parse( LastEdit.c_str() );
		 rapidjson::Value& wiki = replyJSONa["query"]["recentchanges"][0];
		string type = wiki["type"].GetString();
		string page = wiki["title"].GetString();
		string timestamp = wiki["timestamp"].GetString();
		string response = "Last Wiki " + type + " was on page " + page + " at " + timestamp  + ". \\n ";
		JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
		return JSON;
	}

//Celebrate an update!
	e = ("(lion:version)"); //Be specific for now, this is just for showing off.
	if ( regex_match(text , e) ) {
		string response = "Rarrrrr! I was freshly groomed " __DATE__ " " __TIME__ "! :lion_face: \\n ";
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
	if ( channel == CHAN_WHOISIN || channel == DM_PERCY ) {
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

		e = ("([Ii]n)|([Oo]ut)|(.*([Ii].|[Hh]ave.|)(’m|'m|am|are|[Nn]ow|[Jj]ust) (currently |now |just |got )(in|arriv(ed|ing)|out|here|left|leaving).*)");
		if ( regex_match(text , e)
				|| regex_match(text , regex("([Nn]ow |[Aa]bout (to |)|)([Bb]ack|[Hh]ead(ing|)) ([Ii]n|[Oo]ut)"))
				)
		{
			signoutWait_timer.cancel();
			signinWait_timer.cancel(); //Cancel the 'wait for someone to sign in after door unlocked' set in doorbot.cpp
			string response = slack::amendlog( text, user );
			if ( (rand() % 100) > 1 ) {
				string emojis[] = {
						"bowtie",
						"thumbsup",
						"lion_face",
						"grinning",
						"white_check_mark",
						"wave"
				};
				int size = ((&emojis)[1] - emojis);
				JSON = slack::reaction(channel , event , emojis[useemoji.random(size)]);
				if ( JSON == "{\"ok\":true}" ) { return ""; }
			}
			JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" , \"thread_ts\" : \"" + event + "\" } " ;
			return JSON;
			//JSON = " { \"channel\" : \"" + channel + "\" , \"text\" : \"" + response + "\" , \"type\" : \"message\" } " ;
		}
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


