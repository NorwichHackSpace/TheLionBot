/*******************************************************************************
 The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation
 Name        : database.hpp
 Authors     : Alan Percy Childs
 Version     :
 *******************************************************************************/
/*
	Database code should rely on SQLite3.
	Try and use SQL as storage in case of program crashes or restarts!
 */

#ifndef DATABASE_HPP_
#define DATABASE_HPP_

#include <sqlite3.h> //Main import!

class sqlite {

	private:
			sqlite3 *_db; //Database connection object
			int _rc;
			char *_zErrMsg = 0;

			static int callback(void* _shared, int argc, char **argv, char **azColName) {
			   int i;
			   std::string *_json = (std::string *)_shared;
			   if ( _json->empty() ) { _json->append("  { "); }
			   else { _json->append(",  { "); }
			   for(i = 0; i<argc; i++) {
				  std::string col = azColName[i];
				  std::string val = argv[i] ? argv[i] : "NULL";
				  if (i) { _json->append(", \""); }
				  else { _json->append("\""); }
				  _json->append(col);
				  _json->append("\": \"");
				  _json->append(val);
				  _json->append("\" ");
			   }
			   _json->append("} ");
			   return 0;
			}

			int open() {
				   _rc = sqlite3_open("slack-stats.db", &_db);
				   if( _rc ) {
					  std::cerr << "Database  : Load failed" << std::endl;
					  return 1;
				   } else {
					  std::cout << "Database  : Opened successfully" << std::endl;
				   }
				   return 0;
			}

			int close() {
				sqlite3_close(_db);
				return 0;
			}

	public:
			sqlite() {
				open();
			}
			~sqlite() {
				close();
			}

			rapidjson::Document exec (std::string sql) {
				   /* Execute SQL statement */
				   const char * _sql = sql.c_str(); //sqlite3_exec requires const char

				   rapidjson::Document json;

				   std::string _json; //Buffer for making JSON
				   //_json = "{ \"sql\": [ ";
				   _rc = sqlite3_exec(_db, _sql, callback, &_json, &_zErrMsg); // Forth here is first argument to callback

				   if( _rc != SQLITE_OK ){
					  std::string err = "{ \"error\": { \"state\" : true, \"msg\": \"" + std::string(_zErrMsg) + "\" } }";
					  sqlite3_free(_zErrMsg);
					  std::cout << "Database  : Error: " << err << std::endl;
					  json.Parse(err.c_str());
				   } else {
					   //_json.append(" ] }");
					   _json = "{ \"sql\": [ " + _json + " ] }";
					   std::cout << "Database  : JSON: " << _json << std::endl;
					   if ( json.Parse(_json.c_str()).HasParseError() ) {
						   std::string err = "{ \"error\": { \"state\" : true, \"msg\": \"Unable to parse string to JSON.\" } }";
						   std::cout << "Database  : Error: " << err << std::endl;
					   }
				   }
				   return json;
			}

};

extern sqlite database; //Officially started in TheLionBot.cpp, we only need one Database connection globally.

//Do not add code below this line
#endif /* DATABASE_HPP_ */
