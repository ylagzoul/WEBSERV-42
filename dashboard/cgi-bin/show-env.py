#!/usr/bin/env python3

import os
import sys
import html
import urllib.parse

# 1. Parse incoming parameters safely (Handling both GET and POST)
method = os.environ.get("REQUEST_METHOD", "GET").upper()
query_string = os.environ.get("QUERY_STRING", "")

# Read POST payload if applicable
post_data = ""
if method == "POST":
    try:
        content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    except ValueError:
        content_length = 0
    if content_length > 0:
        post_data = sys.stdin.read(content_length)

# Combine GET & POST data into a single parameters dictionary
get_params = urllib.parse.parse_qs(query_string)
post_params = urllib.parse.parse_qs(post_data)
combined_form = {**get_params, **post_params}

# Define CGI Environment variables to display
env_info = [
    ("REQUEST_METHOD", "HTTP method used (GET, POST, DELETE...)."),
    ("REQUEST_URI", "Original requested URI."),
    ("SCRIPT_NAME", "Path of the CGI script."),
    ("QUERY_STRING", "Everything after '?'._"),
    ("CONTENT_TYPE", "Type of submitted data."),
    ("CONTENT_LENGTH", "Size of the request body in bytes."),
    ("SERVER_PROTOCOL", "HTTP version used."),
    ("SERVER_NAME", "Server hostname."),
    ("SERVER_PORT", "Listening port."),
    ("REMOTE_ADDR", "Client IP address."),
]

# Send HTTP Headers
print("Content-Type: text/html; charset=utf-8\r\n\r\n")

# Send HTML Content
print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CGI Environment Demo</title>
    <style>
        :root {{
            --bg-color: #f8fafc;
            --card-bg: #ffffff;
            --text-main: #1e293b;
            --text-muted: #64748b;
            --primary: #0f172a;
            --accent: #10b981;
            --border: #e2e8f0;
            --bg-code: #f1f5f9;
        }}

        * {{
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }}

        body {{
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            background-color: var(--bg-color);
            color: var(--text-main);
            line-height: 1.6;
            padding: 2rem 1rem;
        }}

        .container {{
            max-width: 900px;
            margin: 0 auto;
        }}

        header {{
            margin-bottom: 2.5rem;
            text-align: center;
        }}

        h1 {{
            font-size: 2.25rem;
            color: var(--primary);
            margin-bottom: 0.5rem;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 0.75rem;
        }}

        .subtitle {{
            color: var(--text-muted);
            font-size: 1.1rem;
        }}

        .card {{
            background: var(--card-bg);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 1.75rem;
            margin-bottom: 2rem;
            box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.05), 0 2px 4px -2px rgba(0, 0, 0, 0.05);
        }}

        h2 {{
            font-size: 1.4rem;
            margin-bottom: 1.25rem;
            color: var(--primary);
            border-bottom: 2px solid var(--border);
            padding-bottom: 0.5rem;
        }}

        /* Badge design for Request Method */
        .badge {{
            display: inline-block;
            padding: 0.25rem 0.75rem;
            font-size: 0.85rem;
            font-weight: 700;
            border-radius: 9999px;
            text-transform: uppercase;
        }}
        .badge-get {{ background-color: #dbeafe; color: #1e40af; }}
        .badge-post {{ background-color: #d1fae5; color: #065f46; }}
        .badge-other {{ background-color: #f1f5f9; color: #334155; }}

        /* Table Design */
        table {{
            width: 100%;
            border-collapse: collapse;
            margin-top: 0.5rem;
            font-size: 0.95rem;
        }}

        th, td {{
            padding: 0.85rem 1rem;
            text-align: left;
            border-bottom: 1px solid var(--border);
        }}

        th {{
            background-color: #f8fafc;
            color: var(--text-muted);
            font-weight: 600;
            text-transform: uppercase;
            font-size: 0.75rem;
            letter-spacing: 0.05em;
        }}

        tr:hover td {{
            background-color: #fdfdfd;
        }}

        code {{
            font-family: SFMono-Regular, Consolas, "Liberation Mono", Menlo, monospace;
            background-color: var(--bg-code);
            padding: 0.2rem 0.4rem;
            border-radius: 4px;
            font-size: 0.85rem;
            color: #e11d48;
        }}

        .empty-state {{
            color: var(--text-muted);
            font-style: italic;
            text-align: center;
            padding: 1.5rem 0;
        }}

        /* Test Form Styles */
        .form-grid {{
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 1rem;
            margin-bottom: 1rem;
        }}

        @media (max-width: 600px) {{
            .form-grid {{ grid-template-columns: 1fr; }}
        }}

        input[type="text"] {{
            width: 100%;
            padding: 0.75rem;
            border: 1px solid var(--border);
            border-radius: 6px;
            font-size: 0.95rem;
            outline: none;
            transition: border-color 0.2s;
        }}

        input[type="text"]:focus {{
            border-color: var(--text-muted);
        }}

        .btn-group {{
            display: flex;
            gap: 0.75rem;
        }}

        button {{
            padding: 0.75rem 1.5rem;
            border: none;
            border-radius: 6px;
            font-size: 0.95rem;
            font-weight: 600;
            cursor: pointer;
            transition: opacity 0.2s;
            flex: 1;
        }}

        button:hover {{
            opacity: 0.9;
        }}

        .btn-primary {{ background-color: #0284c7; color: white; }}
        .btn-success {{ background-color: #059669; color: white; }}
    </style>
</head>
<body>

<div class="container">

    <header>
        <h1>🐍 Python CGI Environment</h1>
        <p class="subtitle">A modern display of live environment variables and payload data.</p>
    </header>

    <div class="card">
        <h2>Environment Variables</h2>
        <table>
            <thead>
                <tr>
                    <th>Variable</th>
                    <th>Value</th>
                    <th>Description</th>
                </tr>
            </thead>
            <tbody>
""")

# Render variables
for key, desc in env_info:
    raw_val = os.environ.get(key, "(not set)")
    
    # Stylize the request method variable specifically
    if key == "REQUEST_METHOD" and raw_val != "(not set)":
        badge_class = f"badge-{raw_val.lower()}" if raw_val in ["GET", "POST"] else "badge-other"
        value = f'<span class="badge {badge_class}">{raw_val}</span>'
    else:
        value = f"<code>{html.escape(raw_val)}</code>" if raw_val != "(not set)" else f'<span style="color: var(--text-muted);">{raw_val}</span>'

    print(f"""
                <tr>
                    <td><code>{key}</code></td>
                    <td>{value}</td>
                    <td>{desc}</td>
                </tr>
    """)

print("""
            </tbody>
        </table>
    </div>

    <div class="card">
        <h2>Submitted Form Data</h2>
""")

if combined_form:
    print("""
        <table>
            <thead>
                <tr>
                    <th>Field</th>
                    <th>Value(s)</th>
                </tr>
            </thead>
            <tbody>
""")
    for key, values in combined_form.items():
        escaped_key = html.escape(key)
        escaped_value = html.escape(", ".join(values))
        print(f"""
                <tr>
                    <td><strong>{escaped_key}</strong></td>
                    <td>{escaped_value}</td>
                </tr>
        """)
    print("""
            </tbody>
        </table>
""")
else:
    print('        <p class="empty-state">No query parameters or form data submitted yet.</p>')

print(f"""
    </div>

    <div class="card">
        <h2>Interactive Sandbox</h2>
        <p style="color: var(--text-muted); margin-bottom: 1.25rem; font-size: 0.9rem;">
            Use this tool to test how this CGI script receives and displays user-submitted inputs.
        </p>
        <form action="{os.environ.get('SCRIPT_NAME', '')}" method="GET" id="sandbox-form">
            <div class="form-grid">
                <input type="text" name="sample_user" placeholder="Your Name (e.g., Jane Doe)" required>
                <input type="text" name="sample_message" placeholder="A brief message..." required>
            </div>
            <div class="btn-group">
                <button type="submit" onclick="document.getElementById('sandbox-form').method='GET'" class="btn-primary">Submit via GET</button>
                <button type="submit" onclick="document.getElementById('sandbox-form').method='POST'" class="btn-success">Submit via POST</button>
            </div>
        </form>
    </div>

</div>

</body>
</html>
""")