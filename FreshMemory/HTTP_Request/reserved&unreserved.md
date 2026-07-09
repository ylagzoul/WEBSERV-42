# HTTP Request — Every Part Explained in Depth

## First: What is an HTTP Request?

HTTP (HyperText Transfer Protocol) is an application-layer protocol — it runs on top of TCP. It's just plain text sent through a TCP socket. When your browser visits `http://example.com/page?id=5`, it opens a TCP socket to port 80 of that server, and sends a carefully formatted text message. That text is the HTTP request.

## The Full Anatomy of an HTTP Request

```
POST /api/users?filter=active HTTP/1.1\r\n
Host: example.com\r\n
Content-Type: application/json\r\n
Content-Length: 47\r\n
Authorization: Bearer eyJhbGciOiJIUzI1NiJ9\r\n
Accept: application/json\r\n
Connection: keep-alive\r\n
User-Agent: Mozilla/5.0 (X11; Linux x86_64)\r\n
\r\n
{"username": "ali", "email": "ali@example.com"}
```

It has 4 structural parts:

```
┌─────────────────────────────────────────┐
│            REQUEST LINE                 │  ← Line 1
├─────────────────────────────────────────┤
│                                         │
│             HEADERS                     │  ← Lines 2 to N
│                                         │
├─────────────────────────────────────────┤
│           BLANK LINE (\r\n)             │  ← Separator
├─────────────────────────────────────────┤
│           BODY (optional)               │  ← Data
└─────────────────────────────────────────┘
```

## Part 1: The Request Line

```
POST /api/users?filter=active HTTP/1.1
^     ^                        ^
│     │                        └── Protocol version
│     └─────────────────────────── Request target (URI)
└───────────────────────────────── HTTP Method
```

### 1A — HTTP Method

Tells the server what action to perform. It's a semantic contract — the server is expected to respect what each method means.

| Method | Meaning | Has Body? | Idempotent? |
|---|---|---|---|
| GET | Retrieve a resource | No | Yes |
| POST | Submit data, create resource | Yes | No |
| PUT | Replace a resource entirely | Yes | Yes |
| PATCH | Partially update a resource | Yes | No |
| DELETE | Remove a resource | No | Yes |
| HEAD | Like GET but response has no body — used to check if resource exists/get metadata | No | Yes |
| OPTIONS | Ask server what methods it supports for a resource | No | Yes |

Idempotent means calling it 10 times has the same effect as calling it once. `DELETE /user/5` is idempotent (user is gone after first call, subsequent calls have no further effect). `POST /users` is not — it would create 10 users.

### 1B — Request Target (URI)

```
/api/users?filter=active&page=2#section
│          │                    │
│          │                    └── Fragment (never sent to server, browser-only)
│          └────────────────────── Query String
└───────────────────────────────── Path
```

**The Path `/api/users`:**

- Identifies the specific resource on the server.
- The server routes this to the right handler/controller in your application.
- Case-sensitive on most servers (Linux filesystems are case-sensitive).

**The Query String `?filter=active&page=2`:**

- Key-value pairs separated by `&`.
- Starts after `?`.
- Used for filtering, sorting, pagination — anything that modifies how you retrieve a resource without being part of the resource's identity.
- Values must be URL-encoded: spaces become `%20` or `+`, special chars like `&` inside a value become `%26`, etc. This is because `&` and `=` are structural characters in the query string itself.

```
/search?q=hello%20world&lang=en
```

**The Fragment `#section`:**

- Never sent to the server — purely browser-side for jumping to an anchor on the page.

### 1C — Protocol Version

```
HTTP/1.1    ← Most common, persistent connections, chunked transfer
HTTP/1.0    ← Legacy, one request per connection
HTTP/2      ← Binary, multiplexed, header compression (not plain text anymore)
HTTP/3      ← Runs over QUIC (UDP-based) instead of TCP
```

HTTP/1.1 introduced `Connection: keep-alive` (persistent connections) — instead of opening a new TCP connection per request, the same TCP socket is reused for multiple requests. This matters greatly for performance.

HTTP/2 is fundamentally different in transport — it's binary framing, not plain text, and can send multiple requests simultaneously over one TCP connection (multiplexing). But from the programmer's perspective the concepts (methods, headers, status codes) are the same.

## Part 2: Headers

Headers are metadata about the request. Each one is:

```
Header-Name: header value\r\n
```

Case-insensitive names, terminated by `\r\n` (carriage return + line feed — HTTP mandates this, not just `\n`). The headers section ends with a blank line (`\r\n` on a line by itself).

Let's go through the most important ones:

### Host (mandatory in HTTP/1.1)

```
Host: example.com
Host: example.com:8080   ← port included only if not default (80/443)
```

Why it exists: virtual hosting. One physical server (one IP) can serve hundreds of websites. When a request arrives at IP `93.184.216.34`, how does the server know which website (example.com vs mysite.com) is being requested? The Host header tells it. Without this, virtual hosting would be impossible.

### Content-Type

```
Content-Type: application/json
Content-Type: application/x-www-form-urlencoded
Content-Type: multipart/form-data; boundary=----WebKitFormBoundary
Content-Type: text/plain; charset=utf-8
```

Tells the server what format the body is in so it knows how to parse it.

| Value | Used For |
|---|---|
| application/json | JSON data (APIs) |
| application/x-www-form-urlencoded | HTML form submission (default) |
| multipart/form-data | File uploads (body split into parts) |
| text/plain | Raw text |
| application/octet-stream | Raw binary |

`boundary=----WebKitFormBoundary` in multipart is a unique string used as a delimiter between parts in the body — more on this in the body section.

### Content-Length

```
Content-Length: 47
```

The exact byte count of the body. Critical because:

- HTTP runs over TCP which is a byte stream with no message boundaries (just like we discussed).
- The server needs to know when the body ends and where the next request begins.
- Without this (or chunked encoding), the server doesn't know when to stop reading.

If Content-Length is wrong (too small), the server truncates your body. If too large, it waits for more data that never comes (hangs).

### Authorization

```
Authorization: Bearer eyJhbGciOiJIUzI1NiJ9.eyJzdWIiOiJ1c2VyMSJ9.abc123
Authorization: Basic dXNlcjpwYXNzd29yZA==
```

Carries credentials to authenticate the request.

**Bearer token** (JWT most commonly): the server validates the token cryptographically, no session lookup needed. The token itself encodes who the user is.

**Basic**: Base64(username:password). Note: Base64 is not encryption — it's just encoding. This is why Basic Auth must always be used over HTTPS.

### Accept

```
Accept: application/json
Accept: text/html, application/xhtml+xml, */*
Accept: image/webp, image/png, */*; q=0.8
```

Tells the server what response formats the client understands. The server uses content negotiation to pick the best format it can serve. `q=0.8` is a quality/preference weight (0 to 1). `*/*` means "anything".

Related headers:

```
Accept-Language: en-US, ar;q=0.9    ← preferred response language
Accept-Encoding: gzip, deflate, br  ← client can handle compressed responses
Accept-Charset: utf-8               ← preferred character set
```

### Connection

```
Connection: keep-alive   ← reuse TCP connection for more requests
Connection: close        ← close TCP connection after this response
```

In HTTP/1.1, `keep-alive` is the default — the TCP connection stays open so the browser can immediately fire the next request (for CSS, images, JS) without a new TCP handshake each time. `close` tells the server to shut it down after responding.

### User-Agent

```
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 Chrome/124.0
User-Agent: curl/7.88.1
User-Agent: MyApp/2.1 (custom HTTP client)
```

Identifies the client software making the request. Servers use this for:

- Analytics (browser breakdown)
- Serving mobile vs desktop versions
- Blocking bots/scrapers
- Debugging ("is this coming from curl or a browser?")

### Cookie

```
Cookie: session_id=abc123; theme=dark; lang=en
```

Key-value pairs the browser stores and automatically attaches to every request to that domain. The server originally set these via `Set-Cookie` in a previous response. Used for sessions, preferences, tracking.

### Referer (yes, misspelled historically)

```
Referer: https://google.com/search?q=http+tutorial
```

The URL of the page that linked to this request. If you click a link on Google that goes to example.com, the request to example.com includes `Referer: https://google.com/...`. Used for analytics and security checks (CSRF protection).

### Cache-Control

```
Cache-Control: no-cache
Cache-Control: max-age=3600
Cache-Control: no-store
```

Controls caching behavior. `no-cache` = must revalidate with server before using cache. `no-store` = don't cache at all. `max-age=3600` = cached response is valid for 3600 seconds.

### Custom Headers

```
X-Request-ID: 550e8400-e29b-41d4-a716-446655440000
X-Api-Version: 2
X-Forwarded-For: 203.0.113.5
```

`X-` prefix was the convention for custom/non-standard headers (now deprecated but still widely used). `X-Forwarded-For` is important in infrastructure — when a load balancer or proxy forwards a request, the server sees the proxy's IP, not the real client's. `X-Forwarded-For` carries the original client IP.

## Part 3: The Blank Line

```
\r\n
```

This is the separator between headers and body. It is literally just a blank line — a `\r\n` with nothing before it. This is structural — the server's HTTP parser reads headers line by line until it sees this blank line and knows the body starts next. This is why HTTP parsing in C looks like:

```cpp
// Read until \r\n\r\n — that's the end of headers
while ((pos = buffer.find("\r\n\r\n")) == string::npos) {
    recv(fd, ...);
}
// Everything after that position is the body
```

## Part 4: The Body

The body carries the actual data payload. Its format depends entirely on Content-Type.

### JSON body (most common in APIs)

```
Content-Type: application/json
Content-Length: 47

{"username": "ali", "email": "ali@example.com"}
```

Plain JSON text. Server parses it as JSON.

### URL-encoded form body (HTML forms default)

```
Content-Type: application/x-www-form-urlencoded
Content-Length: 33

username=ali&email=ali%40example.com
```

Same syntax as query string but in the body. `@` is encoded as `%40` because `@` is a special character. This is what a browser sends when you submit a basic `<form>` without file upload.

### Multipart body (file uploads)

```
Content-Type: multipart/form-data; boundary=----Boundary7MA4

------Boundary7MA4\r\n
Content-Disposition: form-data; name="username"\r\n
\r\n
ali\r\n
------Boundary7MA4\r\n
Content-Disposition: form-data; name="avatar"; filename="photo.jpg"\r\n
Content-Type: image/jpeg\r\n
\r\n
<binary JPEG data here>
------Boundary7MA4--
```

The body is split into parts, each separated by the boundary string. Each part has its own mini-headers (Content-Disposition, Content-Type). This is the only way to upload binary files through HTTP forms because URL-encoding binary is extremely inefficient.

## How It All Flows Over TCP

Putting it together — what actually travels through the socket:

1. Browser calls `connect()` to `93.184.216.34:80`
2. TCP 3-way handshake completes
3. Browser calls `send()`:

```
"GET /index.html HTTP/1.1\r\n"
"Host: example.com\r\n"
"Accept: text/html\r\n"
"Connection: keep-alive\r\n"
"\r\n"
```

4. Server calls `recv()`, reads until `\r\n\r\n`
5. Server parses: method=GET, path=/index.html, headers={...}
6. Server calls `send()` with the HTTP response

The entire request is just a string of bytes flowing through the TCP socket we've already covered — HTTP is literally a text formatting convention on top of raw TCP.

> Want me to go deeper on HTTP responses (status codes, response headers, body) next, or move to how you'd parse an HTTP request yourself in C++ (which directly connects all this to your socket recv() calls)?

---

# Query String: Modifying How You Retrieve a Resource

The key idea is the distinction between what you're requesting and how you want it delivered.

## The Resource vs. The Parameters

Think of a resource as a thing that exists on the server — a collection of users, a list of products, an article. The path identifies that thing:

```
/api/users        ← the "users" resource
/api/products     ← the "products" resource
/api/articles     ← the "articles" resource
```

The resource itself doesn't change based on how you ask for it. But your view of it can change — you might want only some of it, in a certain order, at a certain page. That's what the query string is for.

## The Three Concepts

### 1. Filtering — "Give me only the subset I care about"

```
/api/users?status=active
/api/users?status=active&country=MA
/api/products?category=electronics&price_max=500
```

The resource is still `/api/users` — all users. But you're telling the server: don't give me everything, only rows that match these conditions. Think of it as a WHERE clause in SQL:

```sql
SELECT * FROM users WHERE status = 'active' AND country = 'MA';
```

The query string `?status=active&country=MA` is essentially that WHERE clause, expressed in the URL.

### 2. Sorting — "Give me the same data but in a different order"

```
/api/products?sort=price&order=asc
/api/users?sort=created_at&order=desc
```

The data is the same set of products or users — nothing is added or removed. You're just changing the order in which the server returns them. SQL equivalent:

```sql
SELECT * FROM products ORDER BY price ASC;
```

Different sort values don't mean different resources — it's the same resource, presented differently.

### 3. Pagination — "Give me one page of a large result set"

```
/api/users?page=3&limit=20
/api/products?offset=40&limit=20
```

Imagine `/api/users` has 10,000 users. You can't send all of them at once — it would be slow and wasteful. So you split the result into pages and the client asks for one page at a time:

```
?page=1&limit=20  → users 1–20
?page=2&limit=20  → users 21–40
?page=3&limit=20  → users 41–60
```

Again, the resource (`/api/users`) is the same. You're just controlling which chunk of it you want right now.

## "Without Being Part of the Resource's Identity"

This is the critical design principle. A resource's identity is its path — the canonical address of the thing itself:

```
/api/users/42       ← identity of user with id 42
/api/articles/99    ← identity of article 99
```

Changing the path means a different resource. But changing the query string means the same resource, different view:

```
/api/users              ← the users collection
/api/users?status=active        ← same collection, filtered view
/api/users?sort=name            ← same collection, sorted view
/api/users?page=2&limit=10      ← same collection, second page
```

A practical way to think about it:

> If you removed the query string, would the path still point to the same fundamental thing? Yes → query string belongs there. No → it should be in the path.

**Path** → changes what you're addressing:

```
/api/users/42      ← a specific user (different resource than /api/users/43)
```

**Query string** → changes how you're retrieving it:

```
/api/users/42?fields=name,email   ← same user, just fewer fields returned
```

## A Concrete Real-World Example

Imagine you're building a product listing page for an e-commerce site:

```
GET /api/products?category=phones&brand=samsung&price_min=200&price_max=800&sort=rating&order=desc&page=2&limit=12
```

Breaking this down:

| Parameter | Type | What it does |
|---|---|---|
| category=phones | Filter | only phones |
| brand=samsung | Filter | only Samsung |
| price_min=200 | Filter | at least $200 |
| price_max=800 | Filter | at most $800 |
| sort=rating | Sort | order by rating |
| order=desc | Sort | highest rating first |
| page=2 | Pagination | second page |
| limit=12 | Pagination | 12 items per page |

The resource is still just `/api/products` — the products collection. All those parameters are just shaping your window into that data. The server might translate this directly into:

```sql
SELECT * FROM products
WHERE category = 'phones'
  AND brand = 'samsung'
  AND price BETWEEN 200 AND 800
ORDER BY rating DESC
LIMIT 12 OFFSET 12;
```

**The bottom line:** the path tells the server what thing you want, the query string tells it how you want it delivered. They're separate concerns, and keeping them separate makes APIs cleaner, more predictable, and easier to use.

---

## Follow-up: "ahh you said it is the same page just it scroll or something like this"

No, not scrolling — good question, let me clarify this properly.

### Pagination is NOT Scrolling

Scrolling happens on the frontend (browser/app). Pagination happens on the backend (database + server). They are two different layers.

### What Pagination Actually Means

Imagine you have a database with 10,000 users. When a client requests `/api/users`, the server does NOT fetch all 10,000 rows and send them — that would be:

- Slow (huge query)
- Heavy on memory (server loads 10,000 objects)
- Wasteful on network (sending megabytes the user might never scroll to)

Instead, the server fetches and sends only a small chunk — say 20 rows — and the client asks for the next 20 when it needs them.

```
Request 1: GET /api/users?page=1&limit=20  → server queries rows 1–20   → sends 20 users
Request 2: GET /api/users?page=2&limit=20  → server queries rows 21–40  → sends 20 users
Request 3: GET /api/users?page=3&limit=20  → server queries rows 41–60  → sends 20 users
```

Each one is a completely separate HTTP request — a new TCP send/recv cycle. The server is not "remembering" where you were. Each request independently tells the server exactly which chunk to fetch.

### The Scrolling Connection (Infinite Scroll)

Now — scrolling on the frontend can trigger pagination requests on the backend. This is called infinite scroll:

```
User scrolls down on Instagram
        ↓
JavaScript detects: "user is near the bottom"
        ↓
Frontend sends: GET /api/posts?page=2&limit=10
        ↓
Server queries database: LIMIT 10 OFFSET 10
        ↓
Server returns 10 more posts as JSON
        ↓
JavaScript appends them to the page
        ↓
User keeps scrolling → repeat
```

So the scrolling is frontend behavior — pagination is the backend mechanism that powers it. They're not the same thing — scrolling is just one UI pattern that uses pagination behind the scenes.

### Other UI Patterns That Use Pagination

Pagination on the backend can power completely different UI patterns:

**Classic page buttons (Google Search style):**

```
[1] [2] [3] [4] ... [50]   ← user clicks a page number
```

Each click = new request with `?page=N`

**Load more button (Reddit style):**

```
[Load More]   ← user clicks button
```

Click = new request with `?page=N+1`, results appended below

**Infinite scroll (Instagram/Twitter style):**

```
User scrolls → JS auto-fires request → results appended silently
```

All three use the exact same backend pagination — `page` and `limit` in the query string, `LIMIT` and `OFFSET` in SQL. The UI on top is just a design choice.

### The Simple Summary

```
Scrolling  →  a visual/UI thing  →  happens in the browser
Pagination →  a data/backend thing → happens in the server + database
```

Pagination is the server saying: "I'm not giving you all 10,000 rows at once — ask me in chunks." What the frontend does with those chunks (buttons, scroll, load more) is a separate decision entirely.

---

## Follow-up confirmation: Same Path, Different Content

Exactly! You got it. Let me confirm and deepen that understanding.

### Same Path, Different Content

The path is the address of the resource — it never changes:

```
/api/users        ← always points to "the users resource"
```

The query string changes what you get back from that same resource:

```
/api/users?status=active        → returns active users
/api/users?status=inactive      → returns inactive users
/api/users?status=active&page=2 → returns page 2 of active users
/api/users?sort=name            → returns all users sorted by name
```

Same door, different request each time.

### Compare This to When the Path Actually Changes

Path changes mean you are going to a completely different resource:

```
/api/users          → the users collection
/api/users/42       → one specific user (id = 42)
/api/products       → completely different resource
/api/orders         → another different resource
```

So the rule is:

```
PATH changes    →  different resource (different thing)
QUERY changes   →  same resource, different view of it
```

### A Real Life Analogy

Think of a library:

```
The library building      =  your server
A specific bookshelf      =  the path  (/api/users)
"show me only red books"  =  the query (?color=red)
"sorted by author name"   =  the query (?sort=author)
```

You are standing at the same shelf every time — but asking for books in different ways. The shelf (path) didn't change. Your filter (query) did.

### What the Server Does With This

```
Request 1:  GET /api/users?status=active
Request 2:  GET /api/users?status=inactive
Request 3:  GET /api/users?status=active&sort=name&page=2
```

In the server, all three hit the exact same handler/function:

```cpp
// ONE function handles all three requests
if (path == "/api/users") {
    // but reads different params each time
    auto params = parseQuery(query);
    // → different output, same path
}
```

In Express (real world):

```js
// ONE route handles all variations
app.get('/api/users', (req, res) => {
    const status = req.query.status; // changes per request
    const page   = req.query.page;   // changes per request
    // same function, different output
});
```

### The Full Picture

```
URL:  /api/users?status=active&sort=name&page=2
       │                │
       │                └──  Query string → changes the OUTPUT
       │                     same function runs every time
       └── Path → always routes to the same handler
```

So yes — same path, same handler, same function — but the query string makes the response dynamic. That is literally the entire point of query strings: one endpoint, infinite variations of output, without needing to create a new path for every possible combination.

---

# Reserved and Unreserved Characters (RFC 3986)

Good question — this comes from the URI specification that HTTP relies on (originally RFC 1738/1808, later unified and clarified in RFC 3986, "Uniform Resource Identifier (URI): Generic Syntax"). HTTP itself doesn't redefine character rules — it borrows them from the URI spec, since a URL is just a URI.

## The Core Idea

A URI is just a string of characters, but not every character can appear "as-is" inside one. Some characters have structural meaning (they separate parts of the URI), and others are just data. The spec splits all allowed characters into two groups:

## Unreserved Characters

These are safe to use literally, anywhere, with no special meaning:

```
A-Z  a-z  0-9  -  .  _  ~
```

That's it — letters, digits, hyphen, dot, underscore, tilde. They represent themselves and nothing else. You never need to encode them.

## Reserved Characters

These have special/structural meaning in a URI — they're used to delimit different components:

```
: / ? # [ ] @        ← "gen-delims" (generic delimiters)
! $ & ' ( ) * + , ; = ← "sub-delims" (sub-delimiters)
```

| Character | Structural role |
|---|---|
| `:` | separates scheme from rest (`http:`), or host from port |
| `/` | separates path segments |
| `?` | starts the query string |
| `#` | starts the fragment |
| `@` | separates userinfo from host |
| `&` `=` | separates query parameters and key/value pairs |

## Why the Distinction Matters

If a reserved character appears as data (not as a delimiter), it becomes ambiguous — the parser can't tell if it's structural or part of your value. That's the whole reason percent-encoding (`%XX`) exists.

Example:

```
/search?q=100%&off  ← is "%" part of the value, or something else?
/search?q=cats&dogs=true  ← if a value itself contains "&", parsing breaks
```

So if your data contains a reserved character, you must encode it:

```
& → %26
= → %3D
? → %3F
/ → %2F
: → %3A
```

Example: a value that legitimately contains an `@`:

```
/search?email=ali%40example.com
```

Here `%40` is the encoded `@` — because if it appeared raw, it might confuse the parser or just be visually ambiguous with the "userinfo@host" syntax used elsewhere in URIs.

## Quick Mental Model

```
Unreserved  →  safe anywhere, means itself, never needs encoding
Reserved    →  means something structural UNLESS it's your literal data,
               in which case you must percent-encode it
```

So when you see `%20` for space, `%40` for `@`, `%26` for `&` — that's exactly this rule in action: a character that would otherwise be misread as structure is being escaped so it's treated as plain data instead.
