/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : TheLionBot.cpp
 Authors     : Alan Percy Childs
 Version     :
*******************************************************************************/
 /*
   Only modify this file to include
    - function definitions (prototypes)
    - include files
    - extern variable definitions
   In the appropriate sections
 */

#ifndef CPP_THELIONBOT_HPP_
#define CPP_THELIONBOT_HPP_

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "LUrlParser/LUrlParser.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

#include "root_certificates.hpp" //Imported from Boost examples, might not be up to date.

#include <rapidjson/reader.h>
#include <rapidjson/document.h>

#define WORD_SEPERATORS "., \"'()[]<>;:-+&?!\n\t"

//TheLionBot.cpp
std::string usertoname( std::string user) ;

//Handle responses
class response {
	private:
		int lastmsg = -1;
	public:
		int random(int size) {
			int msgnum = (rand() % size);
			while (msgnum == lastmsg) {
				msgnum = (rand() % size);
			}
			lastmsg = msgnum;
			return msgnum;
		};
};

//doorbot.cpp
std::string doorstatus();

//whoami.cpp
std::string whoami( std::string user );

//whoisin.cpp
std::string amendlog( std::string text, std::string user );
std::string amendlog( unsigned int population );
std::string occupancy();

std::string slackMsgHandle( std::string text, std::string user, std::string channel, std::string event_ts );
void slackDoorbotHandle( std::string text, std::string user, std::string channel, std::string event_ts );

//Do not add code below this line
#endif /* CPP_THELIONBOT_HPP_ */
