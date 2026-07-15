*This project has been created as part of the 42 curriculum by mradouan, abouhki, ylagzoul*

# Webserv


## Description

### What is Webserv:

Webserv is an HTTP/1.0 web server implemented in C++98. and it is designed to handle HTTP requests from clients, and process them according to the server configuration and generate appropriate HTTP responses.

### Main goal

The goal of this project is to understand how real web servers operate internally. The project is about handling client connections, parsing HTTP requests routing requests according to a configuration file, serving static files, executing cgi scripts and generating appropriate HTTP response. All while remaining compatible with the HTTP/1.0 protocol.


## Instructions

### Prerequisites

you need the following installed locally:

- C++98 compatible compiler
- Make
- Linux OS
- Telnet (optional, for manual HTTP request testing)

### Compilation

```bash
make
```
### Run server

```bash
./webserv cfg/conf.nginx
```

### Test server

Open your **browser** and navigate to:

```
http://localhost:<port>
```
Optionally you can test the server by sending custom HTTP requests using **Telnet** :

```bash
telnet localhost PORT
```

### Clean project 

```bash
make fclean
```

## Resources

### Documentation

- **HTTP/1.0:** RFC 1945 - [Hypertext Transfer Protocol -- HTTP/1.0](https://www.google.com)

- **CGI:** RFC 3875 - [The Common Gateway Interface (CGI) Version 1.1](https://www.rfc-editor.org/rfc/rfc3875)

- **URI:** RFC 3986 - [Uniform Resource Identifier (URI): Generic Syntax](https://www.rfc-editor.org/rfc/rfc3986)
###

### Tutorials

- TutorialsPoint — HTTP Tutorial
https://www.tutorialspoint.com/http/index.htm

### Articles

- GeeksforGeeks – Socket in Computer Network
  https://www.geeksforgeeks.org/computer-networks/socket-in-computer-network/

### Videos

- "C++ Network Programming Part 1: Sockets"

  https://www.youtube.com/watch?v=gntyAFoZp-E&t=4243s

- "The Difference Between Cookies and Sessions and Their Importance in Web Development"

  https://www.youtube.com/watch?v=hUQPu-RWJ_U


### AI Usage

  The AI tools **Claude** and **ChatGPT** were used as learning and debugging tools throughout the project. and they used for :

  - Undestanding concepts related to web servers, including HTTP, TCP, sockets, CGI, cookies, sessions, and various C++ features.

  - Assisting with debugging C++ implementation issues such as request parsing and CGI handling

  - Generating custom error pages, CGI scripts, static websites and test files to validate CGI execution.
  
  - Improve the project's **README** including its structure, wording and grammar and formatting.
