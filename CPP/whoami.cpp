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

std::string whoami( string user ) {
	string response;
	//Dan Robertson
	if (user == "UCH3TAE7J") {
		string responses[] = {
					"Dan. Always Dan. Nothing else.",
					"Big Dan",
					"Mr Robertson",
					"The DR",
					"The great Dan. Master of tweaking. Slayer of rule zero and chief 3D printing breakinger.",
					"DAN.",
					"I know what your not."
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
				"The real question, " + usertoname(user) + ", is who are the mice?",
				"You are one in a million," + usertoname(user) + ".",
				"Can I call you " + usertoname(user) + "?",
				usertoname(user)
		};
		int size = ((&responses)[1] - responses);
		int random = rand() % size;
		response = responses[random];
	}
	return response;
}
