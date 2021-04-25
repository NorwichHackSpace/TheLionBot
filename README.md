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

With a Raspberry Pi, you might need a newer Boost than in the default Repo...
`git clone --recursive https://github.com/boostorg/boost.git`
`cd boost`
`./bootstrap.sh --prefix=/usr`
`sudo ./b2 install`


## Build Commands

You probably want the `cd /release/` and `make`.
