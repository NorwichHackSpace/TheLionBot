# TheLionBot
The Lion: Norwich Hackspace's very own bot for Slack and Lion House automation.

This bot is highly customized, and probably won't benifit other groups other than a how-to reference.

## Build Dependancies

C++ Linker command... 
`g++ -L/usr/local/ssl/include -L/usr/local/ssl/lib -o "TheLionBot"  ./CPP/TheLionBot.o   -lssl -lpthread -lcrypto`

##Linked Libs
- Boost 1.70+
-- Beast
- RapidJSON 1.1+
- pThread

With Debian/Ubuntu you can cheat and do a...
`sudo apt install gcc libboost-dev rapidjson-dev libssl-dev`