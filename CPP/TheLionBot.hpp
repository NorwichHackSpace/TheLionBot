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
#include <boost/bind.hpp>

#include "root_certificates.hpp" //Imported from Boost examples, might not be up to date.
#include "simpleini/SimpleIni.h" //Loads and handles the configuration files, e.g. anything not stored in the MySQL or MariaDB database
#include "LUrlParser/LUrlParser.h"

#define INI_PATH "TheLionBot.conf"

#define SLACK_TIMEOUT "30" //Minutes (420 minutes = 7 hours)
#define WIKI_POLL "420" //Seconds

#define WORD_SEPERATORS "., \"'()[]<>;:-+&?!\n\t"

#define BUGLINE //std::cout << "PASSED LINE " << __LINE__ << " inside " << __FILE__ << std::endl;

#define USER_PERCY "U0V2J8FRA"
#define USER_DOORBOT "UTDJ1DGUU"
#define BOT_DOORBOT "BTDJ1DFGC"
#define DM_PERCY "D81AQQPFT"
#define CHAN_RANDOM "C0U8Y6BQW"
#define CHAN_DOORSTATUS "CUQV9AGBW"
#define CHAN_LION_STATUS "C01KZGJQ9NH"
#define CHAN_WIKI "CML8QJ3U3"
#define CHAN_GENERAL "C0U8Y6ALE"
#define CHAN_WHOISIN "C2CJR4V63"
#define CHAN_CRYPTO "C01UF8FCHBR"


extern CSimpleIniA settings;

extern boost::asio::deadline_timer signinWait_timer;
extern boost::asio::deadline_timer signoutWait_timer;
extern boost::asio::deadline_timer idle_timer;

bool isChristmas();

//Do not add code below this line
#endif /* CPP_THELIONBOT_HPP_ */
