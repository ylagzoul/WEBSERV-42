# HTTP, Sockets & the TCP Three-Way Handshake — A Deep Dive

An HTTP request is the message a client (usually a web browser, mobile app, or API client) sends to a web server to ask for a resource or request an action. It is the foundation of communication on the web.

To understand HTTP deeply, it's useful to look at it layer by layer — from what happens when you type a URL all the way down to the bytes traveling over the network.

---

## 1. What is HTTP?

HTTP (HyperText Transfer Protocol) is an application-layer protocol in the TCP/IP networking model.

Its job is simple: **a client asks for something, and a server sends back a response.**

```
Client ------------------> Server
       HTTP Request

Client <------------------ Server
       HTTP Response
```

HTTP itself doesn't care what the data is. It can transfer:

- HTML
- JSON
- Images
- Videos
- PDFs
- CSS
- JavaScript
- XML
- Files

---

## 2. Where HTTP Lives

The Internet is built in layers:

```
Application Layer
-----------------
HTTP, HTTPS, FTP, SMTP

Transport Layer
---------------
TCP, UDP

Internet Layer
--------------
IP

Link Layer
-----------
Ethernet, Wi-Fi
```

HTTP sits on top of TCP. For HTTPS:

```
HTTP
TLS (Encryption)
TCP
IP
Ethernet
```

---

## 3. What Happens When You Visit Google?

Suppose you enter `https://google.com`. The browser performs many steps.

### Step 1: Parse the URL

| Component | Value |
|---|---|
| Protocol | `https` |
| Host | `google.com` |
| Path | `/` |
| Query | none |
| Fragment | none |

### Step 2: DNS Lookup

The browser doesn't know where Google lives, so it asks DNS:

```
google.com  →  142.250.xxx.xxx
```

Now it has the IP address.

### Step 3: TCP Connection

The browser opens a TCP connection. TCP performs the famous 3-way handshake:

```
Client -> SYN
Server -> SYN ACK
Client -> ACK
```

Now both machines are connected.

### Step 4: TLS Handshake (HTTPS)

Before HTTP can begin:

- Exchange certificates
- Negotiate encryption
- Generate session keys

Everything afterward is encrypted.

### Step 5: Browser Sends HTTP Request

```http
GET / HTTP/1.1
Host: google.com
User-Agent: Chrome
Accept: text/html
Accept-Language: en-US
```

This is just plain text (before encryption).

### Step 6: Server Processes Request

Google receives `GET /` and decides:

- Is the user logged in?
- Which page?
- Which language?
- Any cookies?
- Cache?
- Authentication?

Then it generates a response.

### Step 7: HTTP Response

```http
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 5340

<html>
...
```

The browser receives the HTML.

### Step 8: Browser Parses HTML

The browser discovers CSS, JavaScript, images, and fonts. **Each one causes more HTTP requests:**

```
GET /style.css
GET /logo.png
GET /app.js
GET /font.woff
```

One webpage may generate 100+ requests.

---

## 4. Anatomy of an HTTP Request

A request has four major parts:

1. Request Line
2. Headers
3. Blank Line
4. Body

```http
POST /login HTTP/1.1
Host: example.com
Content-Type: application/json
Content-Length: 35

{
  "user":"john",
  "password":"123"
}
```

---

## 5. Request Line

**Format:** `METHOD PATH VERSION`

```
GET /products HTTP/1.1
```

| Part | Value |
|---|---|
| Method | `GET` |
| Path | `/products` |
| Version | `HTTP/1.1` |

---

## 6. HTTP Methods

| Method | Purpose | Example |
|---|---|---|
| **GET** | Retrieve data. Should not modify data. | `GET /users` |
| **POST** | Create something (account, upload, message). | `POST /users` |
| **PUT** | Replace an entire resource. | `PUT /users/10` |
| **PATCH** | Update only specific fields. | `PATCH /users/10` |
| **DELETE** | Delete a resource. | `DELETE /users/10` |
| **OPTIONS** | Ask the server what operations are allowed (useful in CORS). | `OPTIONS /users` |
| **HEAD** | Like GET but returns only headers, no body — useful for checking file size, last modified, content type. | `HEAD /users` |

**PUT example** — the entire object is replaced:

```json
// Old
{ "name": "Ali", "age": 20 }

// New
{ "name": "Sara", "age": 25 }
```

**PATCH example** — only specified fields change:

```json
{ "age": 26 }
```

---

## 7. URL Structure

```
https://example.com:443/products/books?id=5#reviews
```

| Component | Value |
|---|---|
| Protocol | `https` |
| Host | `example.com` |
| Port | `443` |
| Path | `/products/books` |
| Query | `?id=5` |
| Fragment | `#reviews` |

> **Important:** The fragment (`#reviews`) is never sent to the server. It's handled entirely by the browser.

---

## 8. HTTP Headers

Headers are metadata about the request:

```http
GET / HTTP/1.1
Host: example.com
User-Agent: Chrome
Accept: text/html
Authorization: Bearer token
```

### Common Request Headers

| Header | Meaning |
|---|---|
| `Host` | Required in HTTP/1.1 — e.g. `example.com` |
| `User-Agent` | Identifies the client (Chrome, Firefox, curl, Postman) |
| `Accept` | What format the client wants, e.g. `application/json` |
| `Content-Type` | What you're sending: `application/json`, `multipart/form-data`, `text/plain` |
| `Authorization` | Contains credentials, e.g. `Bearer token` |
| `Cookie` | Browser automatically sends stored cookies, e.g. `session=abcd123` |
| `Referer` | Which page linked here (yes, historically misspelled) |
| `Origin` | Used for CORS |

---

## 9. Request Body

Only some methods include a body:

```json
{
  "name": "John",
  "age": 20
}
```

Could also be XML, plain text, binary, or a file upload.

---

## 10. Response Structure

Exactly like requests — Status Line, Headers, Blank Line, Body:

```http
HTTP/1.1 200 OK
Content-Type: application/json

{
  "name": "John"
}
```

---

## 11. HTTP Status Codes

| Range | Meaning | Examples |
|---|---|---|
| **1xx** | Informational | `100 Continue` |
| **2xx** | Success | `200 OK`, `201 Created`, `204 No Content` |
| **3xx** | Redirects | `301 Permanent`, `302 Temporary`, `304 Not Modified` |
| **4xx** | Client errors | `400 Bad Request`, `401 Unauthorized`, `403 Forbidden`, `404 Not Found`, `405 Method Not Allowed`, `409 Conflict`, `429 Too Many Requests` |
| **5xx** | Server errors | `500 Internal Server Error`, `502 Bad Gateway`, `503 Service Unavailable`, `504 Gateway Timeout` |

---

## 12. Statelessness

HTTP is stateless — each request is independent. The server doesn't automatically know who you are because it doesn't remember previous requests.

To maintain state, applications use:

- Cookies
- Session IDs
- JWTs (JSON Web Tokens)
- OAuth tokens

---

## 13. Cookies and Sessions

A cookie is stored by the browser:

```
session=ABC123
```

Every request includes it:

```http
Cookie: session=ABC123
```

The server recognizes: *"Oh, this is Alice."*

---

## 14. Caching

To avoid downloading the same resource repeatedly, HTTP supports caching:

```http
Cache-Control: max-age=3600
ETag: "abc123"
Last-Modified: Tue, 30 Jun 2026 12:00:00 GMT
```

Later, the browser can send:

```http
If-None-Match: "abc123"
```

If unchanged, the server replies `304 Not Modified` — no body is sent, saving bandwidth.

---

## 15. Keep-Alive Connections

Instead of opening a new TCP connection for every request (page, CSS, JS, image), **the same TCP connection can be reused.** This reduces latency significantly.

---

## 16. HTTP/1.1 vs HTTP/2 vs HTTP/3

| Feature | HTTP/1.1 | HTTP/2 | HTTP/3 |
|---|---|---|---|
| Text-based | Yes | Binary framing | Binary framing |
| Multiplexing | No | Yes | Yes |
| Head-of-line blocking at transport | Yes | Yes (TCP-level) | Greatly reduced (QUIC) |
| Transport | TCP | TCP | QUIC (built on UDP) |
| Header compression | No | HPACK | QPACK |

HTTP/2 allows multiple requests and responses to share a single connection efficiently. HTTP/3 uses QUIC, which runs over UDP and reduces connection setup time and the impact of packet loss.

---

## 17. Idempotency

An operation is **idempotent** if performing it multiple times has the same effect as performing it once.

| Method | Idempotent? |
|---|---|
| GET | Yes |
| PUT | Yes |
| DELETE | Usually (deleting an already-deleted resource leaves the end state unchanged) |
| POST | No (sending twice may create two resources) |
| PATCH | Depends on the operation |

This matters for retries after network failures.

---

## 18. REST APIs and HTTP

HTTP is the transport protocol; REST is an architectural style that uses HTTP conventions.

```
GET    /users
POST   /users
GET    /users/10
PATCH  /users/10
DELETE /users/10
```

Responses often use JSON:

```json
{
  "id": 10,
  "name": "Alice"
}
```

---

## 19. Security Considerations

- **HTTPS** encrypts traffic using TLS, protecting data from eavesdropping and tampering.
- **Authentication** verifies who the client is (passwords, API keys, OAuth, JWTs).
- **Authorization** determines what an authenticated user is allowed to do.
- **CSRF** (Cross-Site Request Forgery) tricks a browser into making unintended authenticated requests; defenses include CSRF tokens and SameSite cookies.
- **XSS** (Cross-Site Scripting) injects malicious scripts into pages; defenses include output escaping and a strong Content Security Policy (CSP).
- **CORS** (Cross-Origin Resource Sharing) controls when browsers allow JavaScript on one origin to access resources on another origin.

---

## 20. A Complete Example

**Request:**

```http
POST /login HTTP/1.1
Host: example.com
Content-Type: application/json
Accept: application/json

{
  "email": "alice@example.com",
  "password": "correct horse battery staple"
}
```

**Response:**

```http
HTTP/1.1 200 OK
Content-Type: application/json
Set-Cookie: session=xyz789; HttpOnly; Secure; SameSite=Lax

{
  "message": "Login successful",
  "user": {
    "id": 42,
    "name": "Alice"
  }
}
```

On the next request, the browser automatically includes the cookie:

```http
GET /profile HTTP/1.1
Host: example.com
Cookie: session=xyz789
Accept: application/json
```

The server looks up the session, identifies the user, and returns the profile.

---

## Key Takeaways

- HTTP is an application-layer, request-response protocol.
- A request consists of a request line, headers, an optional body, and a blank line separating headers from the body.
- Common methods include GET, POST, PUT, PATCH, and DELETE.
- HTTP is stateless, so applications use cookies, sessions, or tokens to maintain user state.
- HTTPS adds TLS encryption to protect requests and responses.
- HTTP/2 improves efficiency with multiplexing, while HTTP/3 builds on QUIC to reduce latency and improve resilience to packet loss.
- Understanding HTTP also requires understanding related technologies such as DNS, TCP, TLS, caching, cookies, CORS, and REST APIs, since they work together during nearly every web request.

Once you understand HTTP at this level, the next step is to study how web servers (such as Nginx or Apache), application frameworks (such as Express, Django, or Spring Boot), and browser networking internals implement these concepts in real systems.

---

# HTTP vs Sockets

This is one of the most important questions in networking because it connects application programming with network communication.

**The short answer:** HTTP is a protocol. A socket is a communication endpoint. HTTP messages are usually sent through sockets.

Let's go deeper.

## The Big Picture

Imagine you want to send a letter:

- **HTTP** is the language and rules for writing the letter.
- **TCP** is the postal service.
- **Socket** is your mailbox and the recipient's mailbox.

Without the mailbox (socket), the letter cannot be sent.

```
Application
    │
HTTP Protocol
    │
Socket API
    │
TCP
    │
IP
    │
Internet
```

HTTP doesn't send anything by itself — it relies on sockets.

## What is a Socket?

A socket is an endpoint of communication between two processes, identified by an **IP address + port number**:

```
192.168.1.15:52144
142.250.190.78:443
```

A socket is like an open phone line. When two sockets connect:

```
Client Socket  <------------>  Server Socket
```

they can exchange bytes. **Sockets don't understand HTTP — they only send bytes.**

## HTTP Doesn't Know About Networks

HTTP defines GET, headers, body, status codes — but HTTP never says "send this packet over Ethernet" or "create a TCP connection." Those jobs belong to lower layers.

## The Relationship

Think of HTTP as the contents of a conversation, and sockets as the telephone:

```
You
 │
 │   "Hello!"
 ▼
Telephone
 │
 ▼
Friend
```

Without the telephone, your words never arrive. Without words, the telephone carries meaningless sounds.

**HTTP needs sockets. Sockets don't need HTTP.**

## Socket vs HTTP

| Socket | HTTP |
|---|---|
| Communication endpoint | Communication protocol |
| Sends raw bytes | Defines message format |
| Works at transport interface | Works at application layer |
| Can send anything | Sends structured HTTP messages |
| Can stay open indefinitely | Depends on protocol version |

## Example Without HTTP

A socket can send anything:

```
Client: Hello!!
Server: Hello back!!
```

Or pure binary data:

```
0xFF 0xA1 0x22 0x31
```

Sockets don't care.

## Example With HTTP

```http
GET /hello HTTP/1.1
Host: localhost
```

The socket simply sends these bytes. The server socket receives them, and the web server recognizes this as an HTTP request and processes it.

## How the Browser Uses Sockets

Visiting `https://example.com`:

1. **DNS lookup** — `example.com` → `93.184.216.34`
2. **Create a socket** — `socket()`
3. **Connect it** — `connect()` → `93.184.216.34:443` (TCP connection exists)
4. **TLS handshake** (HTTPS)
5. **Write the HTTP request into the socket** — `GET / HTTP/1.1` / `Host: example.com`
6. **Server reads bytes from its socket**
7. **Server generates a response** — `HTTP/1.1 200 OK` / `<html>...</html>`
8. **Server writes response into its socket**
9. **Browser reads bytes from its socket**

## What Does a Socket Actually Do?

In C:

```c
int sock = socket(AF_INET, SOCK_STREAM, 0);  // create a TCP socket
connect(sock, ...);                          // connect to the server
send(sock, data, length, 0);                 // send bytes
recv(sock, buffer, size, 0);                 // receive bytes
```

`send()` doesn't know HTTP — it simply transmits bytes.

## HTTP is Just Text (HTTP/1.1)

If you open a TCP socket yourself, you can literally type:

```
GET / HTTP/1.1
Host: example.com
```

The server understands it because HTTP/1.1 is a text-based protocol.

## HTTP/2 is Different

HTTP/2 still uses sockets, but instead of text it sends binary frames:

```
Socket → Binary Frames → HTTP/2
```

Same socket, different protocol format.

## HTTP/3

```
HTTP → QUIC → UDP Socket
```

It still uses sockets, but the socket is based on UDP instead of TCP.

## Can Sockets Exist Without HTTP?

Absolutely. Many protocols use sockets: HTTP, HTTPS, SSH, SMTP, FTP, DNS, MQTT, Redis, MySQL, PostgreSQL, Minecraft, WebSocket — all communicate through sockets.

## Can HTTP Exist Without Sockets?

In practice, no. HTTP needs an underlying transport to carry its data:

```
HTTP → TCP Socket            (HTTP/1.1, HTTP/2)
HTTP → QUIC (over UDP Socket) (HTTP/3)
```

## Visual Summary

```
                Browser
                   │
          Build HTTP Request
                   │
                   ▼
         "GET /index.html"
                   │
         Write into Socket
                   │
                   ▼
        ┌──────────────────┐
        │    TCP Socket    │
        └──────────────────┘
                   │
                   ▼
                 TCP → IP → Internet → IP → TCP
                   │
                   ▼
        ┌──────────────────┐
        │   Server Socket  │
        └──────────────────┘
                   │
                   ▼
            Web Server (HTTP)
                   │
                   ▼
            HTTP Response
```

## A Useful Analogy

Think of a socket as a pipe and HTTP as the language spoken through that pipe.

- The **socket** creates and maintains the communication channel.
- **TCP** ensures the bytes arrive reliably and in order (or QUIC does this differently for HTTP/3).
- **HTTP** defines what those bytes mean: methods (GET, POST), headers, status codes, and bodies.

A socket can carry any protocol, not just HTTP. HTTP is simply one of the many protocols that use sockets to move data between a client and a server.

---

# The TCP Three-Way Handshake

The TCP Three-Way Handshake is one of the most fundamental concepts in computer networking. It's the process that establishes a reliable connection between a client and a server before any application data (like an HTTP request) is exchanged.

## Why Do We Need a Handshake?

Imagine calling someone on the phone. If you start talking immediately, you don't know if the other person answered, can hear you, or if you can hear them. So instead:

> **You:** "Hello?"
> **Friend:** "Hello, I hear you."
> **You:** "Great, I hear you too."

Now both sides know the connection works in both directions. TCP does exactly this.

## The Three Steps

```
Client                           Server
   |                                |
   |------------ SYN -------------> |
   |                                |
   | <-------- SYN + ACK ---------- |
   |                                |
   |------------ ACK -------------> |
   |                                |
 Connection Established
```

After these three messages, both sides are ready to exchange data.

### Step 1 — SYN

The client sends a TCP packet with the **SYN** (Synchronize) flag set:

```
Client
  | SYN
  | Sequence Number = 1000
  ▼
Server
```

The SYN packet says: *"I'd like to establish a TCP connection."* It also contains an **Initial Sequence Number (ISN)** — here, `1000`. This number marks the first byte in the client's data stream.

At this point, the client has sent a request but doesn't know whether the server received it.

#### What Is a Sequence Number?

TCP sends data as a stream of bytes. Sending "Hello" (5 bytes), with ISN = 1000:

| Byte | Sequence # |
|---|---|
| H | 1001 |
| e | 1002 |
| l | 1003 |
| l | 1004 |
| o | 1005 |

The first actual data byte starts at 1001 because the SYN itself consumes one sequence number.

Sequence numbers let TCP:

- Detect lost packets
- Detect duplicates
- Reassemble data in order

### Step 2 — SYN + ACK

The server receives the SYN and replies with both **SYN** and **ACK** flags:

```
Sequence Number = 5000
Acknowledgment Number = 1001
```

The acknowledgment number means: *"I successfully received your SYN. I'm now expecting sequence number 1001 next."* The server also sends its own initial sequence number (`5000`) because communication is bidirectional.

#### Why Does the Server Send Its Own SYN?

A TCP connection is full-duplex — both sides send data independently:

```
Client  --------------------> Server
Server  <-------------------- Client
```

Each direction needs its own sequence numbers. So the server says: *"I got your starting number. Here's mine."*

### Step 3 — ACK

The client receives the server's SYN+ACK and replies:

```
ACK
Acknowledgment Number = 5001
```

This means: *"I received your SYN. I'm expecting sequence number 5001 next."* Now the handshake is complete:

```
Client                           Server
Seq = 1000  SYN  ------------->

             <-------------  SYN ACK
             Seq = 5000
             Ack = 1001

ACK Ack = 5001  ------------->

Connection Established
```

## After the Handshake

The client can now send application data. For an HTTP request:

```
Seq = 1001
Length = 200 bytes
```

The server replies `ACK = 1201`, meaning: *"I've received bytes 1001 through 1200. Please send byte 1201 next."* This cumulative acknowledgment is one of the ways TCP provides reliable delivery.

## Why Three Steps?

A common question: *"Why not just two?"*

```
Client -> SYN
Server -> SYN ACK
```

If the final ACK never arrives, the server thinks the connection is established, but the client might have crashed or never received the reply. The third ACK confirms that the client received the server's response and both sides agree the connection is ready.

## TCP Connection States

**Client:**
```
CLOSED → (connect()) → SYN_SENT → (receive SYN+ACK) → ESTABLISHED
```

**Server:**
```
CLOSED → LISTEN → (receive SYN) → SYN_RECEIVED → (receive ACK) → ESTABLISHED
```

## What Is in a SYN Packet?

A TCP packet contains much more than just the SYN flag:

```
Source Port:        53124
Destination Port:   443

Sequence Number:    1000
Acknowledgment:     0

Flags:
    SYN = 1
    ACK = 0
    FIN = 0
    RST = 0

Window Size: 65535

Options: MSS, Window Scaling, SACK, Timestamp
```

Some important fields exchanged during the handshake:

- **MSS** (Maximum Segment Size): the largest TCP payload each side can receive in one segment.
- **Window Scaling**: allows larger receive windows for high-speed networks.
- **SACK** (Selective Acknowledgment): helps recover from packet loss more efficiently.
- **Timestamps**: improve round-trip time measurement and help protect against old duplicate packets.

## What Happens If a Packet Is Lost?

If the SYN is lost, the client waits for a timeout, then retransmits it. If the SYN+ACK is lost, the client still doesn't receive a reply, so it retransmits the SYN — the server recognizes it's a retransmission and responds again. TCP uses timers and retransmissions to cope with packet loss.

## How the Handshake Fits into an HTTP Request

```
Browser
   │
   │ DNS lookup
   │
TCP Three-Way Handshake
   │
TLS Handshake (encryption setup)
   │
HTTP Request
   │
HTTP Response
```

The TCP handshake must happen before the HTTP request can be sent because HTTP needs an established transport connection.

## Visual Timeline

```
Client                                         Server
  |                                               |
  | SYN (Seq=1000) -----------------------------> |
  |                                               |
  | <---------------- SYN+ACK (Seq=5000, Ack=1001)
  |                                               |
  | ACK (Ack=5001) -----------------------------> |
  |                                               |
  | ===== Connection Established =====            |
  |                                               |
  | HTTP GET / --------------------------------> |
  |                                               |
  | <------------------------ HTTP 200 OK         |
```

## Key Ideas to Remember

- **SYN** means "I'd like to start a TCP connection."
- **ACK** means "I received what you sent."
- The handshake takes three messages because both sides must synchronize their sequence numbers and confirm that communication works in both directions.
- No HTTP, HTTPS, or other TCP-based application data is sent until the handshake completes.
- After the connection is established, TCP continues using sequence numbers and acknowledgments to ensure reliable, ordered delivery of data.
