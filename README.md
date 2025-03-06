# Webserv
This is a basic http web server, written in C++ by Ryan Boudwin, Panu Kangas and Patrik Lång as part of the 42 curriculum offered by [Hive Helsinki](https://www.hive.fi/en/), a peer to peer coding school.
## Requirements
* This was written in C++ using the 2020 standard, so you'll need an up to date compiler that supports it (g++ or clang++ should work).
* It was written and tested primarily on Ubuntu Linux and the Google Chrome web browser. It has also been tested on Mac OS X.

## Features
* Supports GET, POST, and DELETE http methods
* Serves static web content
* Supports uploading files using POST
* Uses poll() to ensure we can work properly with non-blocking file descriptors and serve all clients in a timely manner
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
We reference NGINX as inspiration for the configuration of our HTTP server.

You can find examples of different configurations that work with our sample content in the configurations directory.

### Required Settings to Launch the Server
For each server block, the following settings are required to launch the server:
* Port
* IP Address
* Server Name
* Root Location /
Additionally, an index file can be specified in the configuration. By default, index.html will be used.

### Variables for Server Block Configuration:
* listen: The port to listen on (e.g., 8000 - 8999).
* host: The IP address for the server.
* server_name: The server name (e.g., example.com, www.example.com).
* max_client_body_size: The maximum size of a client's body.
* error_page: Allows you to specify custom error pages. If not defined, the server will use the default error pages provided in the sample content.
### Supported Error Codes and Example Usage:
#### The following HTTP error codes can be assigned custom error pages:
```
error_page 400 /home/error/400.html;
error_page 403 /home/error/403.html;
error_page 404 /home/error/404.html;
error_page 405 /home/error/405.html;
error_page 408 /home/error/408.html;
error_page 409 /home/error/409.html;
error_page 411 /home/error/411.html;
error_page 413 /home/error/413.html;
error_page 414 /home/error/414.html;
error_page 431 /home/error/431.html;
error_page 500 /home/error/500.html;
error_page 501 /home/error/501.html;
error_page 503 /home/error/503.html;
error_page 505 /home/error/505.html;
```
If an error page is not explicitly defined, the default pages will be used.

### Location Block Configuration:
The location blocks within the root location are customizable to suit your needs.
The following variables are used within these location blocks:

* root: The path to the location (mandatory).
* methods: The allowed HTTP methods for that location.
* cgi_path_php/python: The path to the CGI interpreter (modify depending on your system).
* dir_listing: Enables or disables directory listing (autoindex).
* upload_dir: Specifies a specific upload directory (optional). The upload directory must be located inside the home (/) directory.
* return: Allows a 307 Temporary Redirect within a location inside the root (/) directory (optional).

#### Important Note:
All of the above-mentioned variables must end with a semicolon (;), except for the server and location blocks, which do not require one.

If the configuration is formatted incorrectly, detailed error messages will be logged in logfile.log. You can view the log file directly in the terminal using:
```
cat logfile.log
```
Additionally, all accepted configurations will be printed in the terminal, allowing you to verify which configurations are active and which addresses they are running on.

## Test instructions
### Unit tests
We provide unit tests, written in Python, that validate our server is using the appropriate HTTP response codes for various situations. For some of these tests we also confirm that the responses are the same nginx would provide in the same situation.
* NOTE: For the included unit tests to work, you will need to have Docker installed so we can launch an nginx container. The webserver itself is not dependent on this.
* You will need to do the following after compiling the server:
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
You should get output that looks like this:
 ```
...
Lifting the server siege...
Transactions:		       29536 hits
Availability:		      100.00 %
Elapsed time:		       50.70 secs
Data transferred:	       80.90 MB
Response time:		        0.00 secs
Transaction rate:	      582.56 trans/sec
Throughput:		        1.60 MB/sec
Concurrency:		        1.89
Successful transactions:       29536
Failed transactions:	           0
Longest transaction:	        1.43
Shortest transaction:	        0.00
```

## Things we learned
