# Understanding URLs: Query Strings, Fragments & Who Fills Them In

## What is a URL?

**URL** stands for **Uniform Resource Locator**. Let's break that down:

- **Uniform** → It follows a standard format.
- **Resource** → Anything on the web (HTML page, image, video, CSS file, API, etc.).
- **Locator** → It tells you where that resource is located.

So a URL is simply: **the address of a resource on the Internet.**

### Example

```
https://example.com/images/logo.png
```

This URL tells the browser: *"Go to the server `example.com` and ask for the file `/images/logo.png`."*

---

## Why Do We Need URLs?

Imagine there were no URLs. Suppose a server contains:

```
index.html
about.html
login.html
style.css
logo.png
video.mp4
```

When the browser connects, how would the server know which file you want? The browser must say something like *"I want this file."* **The URL identifies that resource.**

### A URL is Like a Postal Address

Imagine you want to send a letter. You write:

- Country
- City
- Street
- House Number

That address tells the mail carrier exactly where to deliver the letter. A URL does the same thing for web resources.

---

## Parts of a URL

Consider:

```
https://www.example.com:8080/images/logo.png?size=large#top
```

| Part | Value | What it is |
|---|---|---|
| Scheme | `https://` | Also called the protocol — tells the browser how to communicate (`http`, `https`, `ftp`) |
| Host | `www.example.com` | The domain name — tells the browser which server to contact |
| Port | `:8080` | Tells the browser which service on the server to connect to (HTTP → 80, HTTPS → 443) |
| Path | `/images/logo.png` | Tells the server which resource is requested |
| Query string | `?size=large` | Sends extra information to the server |
| Fragment | `#top` | Used by the browser only — **never sent to the server** |

In your Webserv project, you'll probably do something like:

```cpp
if (request.path == "/images/logo.png")
```

to locate the file.

---

## How a Browser Uses the URL

Suppose you type:

```
http://localhost:8080/login.html
```

The browser analyzes it step by step:

1. **Scheme:** `http` → use HTTP.
2. **Host:** `localhost` → connect to the local machine.
3. **Port:** `8080` → connect to port 8080.
4. **Path:** `/login.html` → ask the server for that file.

The browser then sends:

```http
GET /login.html HTTP/1.1
Host: localhost:8080
```

Notice that the **full URL is not sent**. The browser has already used the scheme, host, and port to establish the connection. The server only needs `/login.html`.

### Why Does the Browser Separate the URL?

Because each part has a different purpose:

| Part | Purpose |
|---|---|
| `https://` | How to communicate |
| `example.com` | Where to connect |
| `/login` | Which resource to request |
| `?user=42` | Extra data for that resource |

### Why Not Just Send the Filename?

Imagine a website with both:

```
/images/logo.png
/users/logo.png
```

If the browser only sent `logo.png`, the server wouldn't know which one you meant. **The path solves this problem.**

---

## Relation to Your Webserv Project

When the browser requests:

```
http://localhost:8080/index.html
```

your server receives:

```http
GET /index.html HTTP/1.1
Host: localhost:8080
```

Your server typically:

1. Reads the request line.
2. Extracts the path (`/index.html`).
3. Maps that path to a file on disk (according to your configuration).
4. Opens the file.
5. Sends it back in the HTTP response.

### Complete Example

The user enters:

```
https://example.com/products/laptop?id=15
```

The browser:

1. Uses `https` to establish a secure connection.
2. Connects to `example.com` on port 443 (the default for HTTPS).
3. Requests the resource `/products/laptop`.
4. Includes the query string `id=15`.

The HTTP request looks like:

```http
GET /products/laptop?id=15 HTTP/1.1
Host: example.com
```

The server sees the path and query, processes the request, and returns the appropriate response.

---

## Summary

A URL is a standardized address that tells the browser:

- **How to communicate** (`http`, `https`)
- **Where to connect** (host and port)
- **What resource to request** (path)
- **Any extra information to send** (query string)

Without URLs, a browser would have no standard way to identify which resource it wants from a server. That's why every HTTP request begins with a URL — or more precisely, the path and optional query extracted from the URL after the browser has connected to the correct server.

---

# Query String vs Fragment — Examples and Why We Need Them

These two parts of a URL are often confused because they both come after the path, but they have completely different purposes.

## 1. Query String (`?`)

A query string sends **extra information to the server**.

**General form:**

```
/path?key=value
/path?key1=value1&key2=value2
```

The important point: **the query string IS sent to the server.**

### Example 1: Search Engine

```
/search?q=cat
```

| Part | Value |
|---|---|
| Path | `/search` |
| Query | `q=cat` |

The browser sends:

```http
GET /search?q=cat HTTP/1.1
Host: example.com
```

The server receives `q = cat` and now knows: *"The user wants to search for cat."*

Without the query string, the server would only receive `/search` — it wouldn't know what to search for.

### Example 2: Login Redirect

```
/login?next=/profile
```

Query: `next=/profile` → the server understands: *"After the user logs in, redirect them to `/profile`."*

### Example 3: Online Store

```
/products?category=laptop&price=1000
```

Query: `category=laptop`, `price=1000` → the server filters the products.

### Example 4: Pagination

```
/articles?page=3
```

The server knows: show page number 3.

### Example 5: Sorting

```
/users?sort=name
```

The server sorts users by name.

### Why Do We Need a Query String?

Imagine there were no query strings. If you only had `/search`, how would the server know what you're searching for? There's nowhere to send `cat`.

**The query string solves this** — it lets you send extra data without changing the path.

---

## 2. Fragment (`#`)

A fragment starts with `#`. Example:

```
/index.html#contact
```

**The critical difference: the fragment is NEVER sent to the server.**

Suppose you type:

```
http://example.com/index.html#contact
```

The browser sends only:

```http
GET /index.html HTTP/1.1
Host: example.com
```

Notice `#contact` is not sent — because **the fragment is used only by the browser**.

### Example 1: Jump to a Section

Suppose an HTML page contains:

```html
<h2 id="contact">Contact Us</h2>
```

The URL is `/index.html#contact`. The browser:

1. Downloads the page.
2. Looks for `id="contact"`.
3. Scrolls directly to it.

The server never knows about `#contact`.

### Example 2: Long Documentation

Suppose `cpp.html` contains sections: Variables, Loops, Functions, Classes. Instead of making four separate pages, you use:

```
cpp.html#variables
cpp.html#loops
cpp.html#functions
cpp.html#classes
```

The browser jumps to the requested section.

### Example 3: Single-Page Applications (SPAs)

Some JavaScript applications use fragments for navigation:

```
/#home
/#settings
/#profile
```

JavaScript reads the fragment and changes the displayed content without reloading the page.

---

## Query String vs Fragment — Side by Side

Suppose the URL is:

```
/products?category=laptop#reviews
```

| Part | Value |
|---|---|
| Path | `/products` |
| Query | `category=laptop` |
| Fragment | `reviews` |

The browser sends:

```http
GET /products?category=laptop HTTP/1.1
Host: example.com
```

Notice `#reviews` is missing — after receiving the page, the browser scrolls to `id="reviews"`.

### Think of It Like Ordering a Book

Imagine a library. The server is the librarian, the browser is you.

- You say: *"I want the book: Programming"* → that's the **path**.
- Then you add: *"English edition."* → that's the **query string** — the librarian needs to know this.
- Finally you say: *"Open directly to Chapter 5."* → that's the **fragment** — the librarian doesn't care. Once you receive the book, *you* open it to Chapter 5.

---

## In Your Webserv Project

Suppose the browser requests:

```http
GET /login?user=youssef HTTP/1.1
```

Your server can parse:

```
path  = "/login"
query = "user=youssef"
```

But if the user visits:

```
http://localhost:8080/login?user=youssef#bottom
```

The browser still sends:

```http
GET /login?user=youssef HTTP/1.1
```

Your Webserv will **never receive `#bottom`**, because the fragment is handled entirely by the browser after it receives the page. This is why web servers parse the path and query string, but they do not parse fragments — they never see them.

---

# In the Query String, Who Sets the Key and Value?

**Short answer:** it depends on the website. Usually, **the developer decides the keys**, and **the browser or JavaScript fills in the values**.

## Example 1: Search Form

```html
<form action="/search" method="GET">
    <input type="text" name="q">
    <button>Search</button>
</form>
```

Notice `name="q"` — **the developer chose the key `"q"`**.

Now the user types `cats`. When they click Search, the browser automatically builds the URL:

```
/search?q=cats
```

| Key | Value |
|---|---|
| `q` (chosen by the developer) | `cats` (typed by the user) |

## Example 2: Login Page (GET)

```html
<form action="/login" method="GET">
    <input name="username">
    <input name="password">
</form>
```

User enters `username = youssef`, `password = 1234`. The browser creates:

```
/login?username=youssef&password=1234
```

Again, `username` and `password` are the keys; `youssef` and `1234` are the values.

> **Note:** Login forms normally use POST, not GET, so passwords don't appear in the URL.

## Example 3: JavaScript

JavaScript can also build a query string:

```javascript
const user = "youssef";
window.location = "/profile?user=" + user;
```

Result: `/profile?user=youssef`. The developer wrote the key `user`; the variable supplies the value `youssef`.

## Example 4: You Type It Yourself

You can also manually type:

```
http://localhost:8080/search?q=webserv
```

Here you created both the key (`q`) and the value (`webserv`).

---

## What Does the Server Do?

Suppose your Webserv receives:

```http
GET /search?q=cat&page=2 HTTP/1.1
```

Your server parses it:

```
Path:  /search
Query: q=cat&page=2
```

Then you can split it into:

| Key | Value |
|---|---|
| `q` | `cat` |
| `page` | `2` |

Now your code can do something like:

```cpp
if (query["q"] == "cat")
{
    // Search for "cat"
}

if (query["page"] == "2")
{
    // Return page 2
}
```

### Think of It Like a Dictionary

Imagine someone gives you:

```
Name: Youssef
Age: 24
City: Khouribga
```

The **keys** are `Name`, `Age`, `City`. The **values** are `Youssef`, `24`, `Khouribga`.

A query string works exactly the same way:

```
name=Youssef&age=24&city=Khouribga
```

---

## In Your Webserv Project

**Your server doesn't decide what the keys are.** The client (browser, JavaScript, or the user) sends something like:

```http
GET /search?q=cpp&sort=date HTTP/1.1
```

Your job is simply to parse it into:

| Key | Value |
|---|---|
| `q` | `cpp` |
| `sort` | `date` |

...and let the rest of your application decide what those keys mean. **The meaning of `q`, `sort`, `page`, `id`, etc., is defined by the website or application developer, not by HTTP itself.**
