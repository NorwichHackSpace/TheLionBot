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

//Nothing to see here
class database {

	public:
		static void open() {
			   sqlite3 *db;
			   int rc;
			   rc = sqlite3_open("slack-stats.db", &db);
			   if( rc ) {
				  std::cout << "Can't open database.";
			   } else {
				  std::cout << "Opened database successfully\n";
			   }
			   sqlite3_close(db);
		}


	private:


};

//Do not add code below this line
#endif /* DATABASE_HPP_ */
