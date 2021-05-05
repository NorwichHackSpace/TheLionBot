/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : fetch.hpp
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

#ifndef FETCH_HPP_
#define FETCH_HPP_

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

namespace fetch { //The simple stuff

	//fetch.cpp
	//std::string http( const char* host, const char* target, const char* port = "80" );
	std::string https( const char* host, const char* target, const char* port = "443" );

}

//Do not add code below this line
#endif /* FETCH_HPP_ */
