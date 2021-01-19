/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : wiki.hpp
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

#ifndef WIKI_HPP_
#define WIKI_HPP_

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/system_executor.hpp>
#include <boost/asio/strand.hpp>

#include <rapidjson/reader.h>
#include <rapidjson/document.h>

namespace wiki { //The simple stuff

	//wiki.cpp
	std::string HTTP( std::string call ) ;
	std::string LastEdit( ) ;
	//extern rapidjson::Document replyJSON;

}

//Do not add code below this line
#endif /* WIKI_HPP_ */
