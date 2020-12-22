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

response iamwho;
std::string slack::whoami( string user ) {
	string response;
	//Craig Bane
	if (user == "U3YFJDXSL") {
		string responses[] = {
				"Apparently, you are a God."
		};
		int size = ((&responses)[1] - responses);
		int random = rand() % size;
		response = responses[random];
	}
	//Dan Robertson
	if (user == "UCH3TAE7J") {
		string responses[] = {
					"Mr Robertson",
					"The great Dan.",
					"Dan.",
					"ooooooooOOOOOOOOOOOooooooooooooooooooo",
					"I'm not random enough to give a non offensive response."
					"3D Printing Room Lead ( 3D Printers, Casting), Core Team member, Runner of Killer Wing Dice and Killer Wing Designs",
		};
		int size = ((&responses)[1] - responses);
		int random = rand() % size;
		response = responses[random];
	}
	//Alan Percy Childs
	else if (user == "U0V2J8FRA") {
		string responses[] = {
				"My great master!",
				"The most excellent bot programmer.",
				"That's percsonal.",
				"Your very percistant.",
				"We all know your real name is Alan.",
				"Well we all call you Percy."
		};
		int size = ((&responses)[1] - responses);
		int random = rand() % size;
		response = responses[random];
	} else {
		string responses[] = {
				"I know you simply as " + user,
				"Until I'm progrtextammed better, I'll call you " + user,
				"All your name are belong to you",
				"That's a very profound question. Who are any of us, really?",
				"You are a computer simulation of a real person using a complex AI virtual matrix to believe that you are real.",
				"The real question, " + slack::usertoname(user) + ", is who are the mice?",
				"You are one in a million," + slack::usertoname(user) + ".",
				"Can I call you " + slack::usertoname(user) + "?",
				"Having some sort of identity crisis?",
				slack::usertoname(user)
		};
		int size = ((&responses)[1] - responses);
		response = responses[iamwho.random(size)];
	}
	return response;
}
