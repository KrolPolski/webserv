# Webserv
This is a basic http web server, written in C++ by Ryan Boudwin, Panu Kangas and Patrik Lang as part of the 42 curriculum offered by [Hive Helsinki](https://www.hive.fi/en/), a peer to peer coding school.
## Requirements
* This was written in C++ using the 2020 standard, so you'll need an up to date compiler that supports it (g++ or clang++ should work).
* It was written and tested primarily on Ubuntu Linux. It has also been tested on Mac OS X.

## Features
* Supports GET, POST, and DELETE http methods
* Serves static web content
* Supports CGI scripts in both PHP and Python for dynamic content
* Robust configuration error detection using regex

## Launch instructions
```
make
./webserv <configuration file> 
```
If you don't specify a configuration file, it will launch with the default configuration file (configurations/complete.conf) that will serve the sample web content we have provided in home/. You can then access it at:
[http://localhost:8080](http://localhost:8080/)
## Configuration instructions

## Test instructions
### Unit tests
We provide unit tests, written in Python, that validate our server is using the appropriate HTTP response codes for various situations. For some of these tests we also confirm that the responses are the same nginx would provide in the same situation., you will need to do the following after compiling the server:
* For the included unit tests to work, you will need to have Docker installed. The webserver itself is not dependent on this.

```
./webserv configurations/deletetest.conf
```
Then in a separate terminal window, you will need to do the following:
```
./nginx_init.sh
cd tests
./unit_tests.py
```
### Stress Test
* For stress testing you will need to install siege (easily achieved using Homebrew in the 42 environment: ```brew install siege``` or ```sudo apt install siege``` on Linux)
* To run a stress test with siege, ensure the web server is already running with the default configuration and then run:
```
cd tests
./besiege_server.sh
```

## Things we learned
