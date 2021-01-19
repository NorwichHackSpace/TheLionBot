# TheLionBot
 
Norwich Hackspace's very own bot for Slack and automation of our Lion House premises.

This bot is highly customized, and probably won't benifit other groups more than a how-to reference.

We will eventally attempt to split code into libs though. Maybe.

## Build Dependancies

- Boost 1.70+
- Beast
- RapidJSON 1.1+
- pThread
- SQLite 3

With Debian/Ubuntu you can cheat and do...

`sudo apt install gcc libboost-dev rapidjson-dev libssl-dev libsqlite3-dev`

## Build Commands

C++ Linker command... 

`g++ -L/usr/local/ssl/include -L/usr/local/ssl/lib -o "TheLionBot"  ./CPP/TheLionBot.o   -lssl -lpthread -lcrypto -lsqlite3`
