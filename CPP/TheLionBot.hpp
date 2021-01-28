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

#define USER_PERCY "U0V2J8FRA"
#define USER_DOORBOT "CMFJQ7NNB"
#define DM_PERCY "D81AQQPFT"
#define CHAN_RANDOM "C0U8Y6BQW"
#define CHAN_DOORSTATUS "CUQV9AGBW"
#define CHAN_LION_STATUS "C01KZGJQ9NH"

#include "LUrlParser/LUrlParser.h"

#define WORD_SEPERATORS "., \"'()[]<>;:-+&?!\n\t"

#define BUGLINE //std::cout << "PASSED LINE " << __LINE__ << " inside " << __FILE__ << std::endl;

//Do not add code below this line
#endif /* CPP_THELIONBOT_HPP_ */
