*This project has been created as part of the 42 curriculum by: dopereir (GitHub: rach3bartmoss), gtretiak (GitHub: gtretiak), and nogioni- (GitHub: nayaraogioni).*

# Webserv

## Description

Webserv is a fully functional HTTP/1.1-compliant web server written in C++98. The goal
of the project is to understand the inner workings of the HyperText Transfer Protocol
(HTTP) by building a server from scratch — without relying on any external or Boost
libraries. The server is non-blocking, event-driven (using poll() or equivalent), chunked transfer decoding, and NGINX-inspired configuration with no external libraries, supports multiple virtual hosts via a configuration file, handles static file serving, file uploads,
directory listing, HTTP redirections, GET, POST, DELETE, and CGI execution (e.g., PHP, Python). It is designed
to remain resilient and available under heavy load.

## Instructions

### Compilation

```bash
make
```
This produces the webserv executable.


### Execution

```bash
./webserv [configuration file]
```

If no configuration file is provided, the server looks for one at a default path
(e.g., conf/default.conf).

Example
```bash
./webserv conf/default.conf
```

### Cleaning

```bash
make clean   # removes object files
make fclean  # removes object files and the binary
make re      # fclean + all
```


## Resources

### HTTP
- RFC 1945 – HTTP/1.0 - https://datatracker.ietf.org/doc/html/rfc1945
- RFC 2616 – HTTP/1.1 - https://datatracker.ietf.org/doc/html/rfc2616
- RFC 7230–7235 – HTTP/1.1 (updated) - https://datatracker.ietf.org/doc/html/rfc7230
- MDN Web Docs – HTTP - https://developer.mozilla.org/en-US/docs/Web/HTTP

### NGINX
- NGINX documentation - https://nginx.org/en/docs/

### CGI
- RFC 3875 – The Common Gateway Interface (CGI) - https://datatracker.ietf.org/doc/html/rfc3875

### POSIX / Sockets
- Beej's Guide to Network Programming - https://beej.us/guide/bgnet/
- POSIX man pages (man 2 socket, man 2 poll, etc.)

### Stress Testing
- siege - https://www.joedog.org/siege-home/
- wrk - https://github.com/wg/wrk
- ApacheBench (ab) - https://httpd.apache.org/docs/2.4/programs/ab.html

### AI Usage
AI (specifically a large language model assistant) was used during this project for the
following purposes:
- Clarifying RFC specifications and HTTP semantics.
- Explaining POSIX API behaviour (poll, socket, accept, etc.).
- Suggesting code structure and separation of concerns.
- Reviewing and explaining edge cases in non-blocking I/O.
- Drafting and refining this README.
AI was not used to directly generate submitted source code files. All implementation
decisions were made and written by the team.

### Features
- Non-blocking, single-poll event loop
- Multiple virtual hosts and ports via configuration file
- GET, POST, DELETE HTTP methods
- Static file serving with MIME type detection
- Directory listing (optional, per route)
- File uploads
- HTTP redirections
- CGI execution (PHP-CGI, Python, etc.)
- Chunked transfer encoding (de-chunking for CGI)
- Default and custom error pages
- Configurable client body size limit
- Compatible with standard web browsers
- Configuration
See conf/default.conf for a fully annotated example configuration file.

---

# Project Division Into Three Parts

Below is a suggested division that keeps workload roughly equal and minimises
interdependencies. The three developers work on well-defined layers that communicate
through agreed interfaces.

---

## Developer 1 (Douglas) — Configuration, CGI & File System

**Responsibility:** Everything that requires reading the config and touching the
filesystem or spawning processes.

- Configuration file parser (server blocks, location blocks, all directives)
- Virtual host support (server name matching)
- CGI execution: `fork`, `execve`, `pipe`, `dup2`, `waitpid`, environment variable
  setup, stdout reading, EOF detection, timeout
- File system operations: `open`, `read`, `write`, `close`, `stat`, `access`,
  `opendir`, `readdir`, `closedir`, `chdir`
- File upload storage (writing uploaded bodies to disk)
- Integration of CGI output back into the HTTP response pipeline
- Default configuration path fallback
- Providing annotated example configuration files and default test pages

**Deliverables:** A config object that Developer 1 and 2 query, plus a CGI subsystem
that Developer 2 calls when the router determines CGI is needed.

---

## Developer 2 (George) — HTTP Layer (Parser, Router, Response Builder)

**Responsibility:** Everything that turns raw bytes into HTTP meaning and back.

- HTTP request parser (request line, headers, body, chunked decoding)
- HTTP response builder (status line, headers, body)
- Router: matches a parsed request to a configured location/route
- Method handlers: GET (static files, directory listing), POST (body collection),
  DELETE (file deletion)
- MIME type detection
- HTTP redirections (3xx responses)
- Default and custom error pages (4xx, 5xx)
- Accurate HTTP status codes
- `Content-Length`, `Transfer-Encoding`, `Connection` header handling

**Deliverables:** A library of classes/functions that takes a raw HTTP request string,
returns a fully formed HTTP response string, based on configuration.

---

## Developer 3 (Nayara) — Core Networking & Event Loop

**Responsibility:** Everything that touches sockets, file descriptors, and the poll loop.

- Socket creation, binding, listening (`socket`, `bind`, `listen`, `accept`)
- Setting sockets non-blocking (`fcntl`)
- The single `poll()` (or `epoll`/`kqueue`) event loop
- Connection lifecycle management (open, read-ready, write-ready, close)
- Raw byte-level reading from and writing to client sockets (`recv`, `send`)
- Buffer management per connection
- Timeout / hang-prevention logic
- Signal handling (`signal`, `kill`)
- Low-level address utilities (`getaddrinfo`, `freeaddrinfo`, `setsockopt`,
  `getsockname`, `getprotobyname`, `htons`, `htonl`, `ntohs`, `ntohl`)

**Deliverables:** A working event loop that accepts connections, reads raw bytes into
per-connection buffers, and writes bytes from per-connection output buffers — without
yet understanding HTTP.

---

**Shared interface (agreed upon early):**
- `Config` / `ServerConfig` / `LocationConfig` structs (Developer 3 owns, others read)
- `Connection` struct / class (Developer 1 owns, Developer 2 reads/writes buffers)
- `HttpRequest` / `HttpResponse` structs (Developer 2 owns, Developer 3 feeds into)

---
