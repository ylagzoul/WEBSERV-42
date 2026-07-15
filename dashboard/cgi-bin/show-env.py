#!/usr/bin/env python3
import os
import sys
import cgi
import html
import time
import cgitb

# Enable browser-friendly traceback debugging
cgitb.enable()

# ==========================================
# 1. CONFIGURATION & DIRECTORY SETUP
# ==========================================
# Relative or absolute directory where files will be saved
UPLOAD_DIR = "./uploads"

# Ensure the target directory exists and has write permissions
if not os.path.exists(UPLOAD_DIR):
    try:
        os.makedirs(UPLOAD_DIR, exist_ok=True)
    except Exception as e:
        # We will log this error gracefully in the UI if directory creation fails
        pass

# ==========================================
# 2. PARSE INCOMING ENVELOPE / SIMULATIONS
# ==========================================
form = cgi.FieldStorage()
error_type = form.getvalue("simulate_error", "normal")

# --- Gateway Simulation Sandbox ---
if error_type == "502":
    # Corrupt headers trigger an upstream 502 Bad Gateway
    print("Corrupt-Header-No-Colon")
    sys.exit(1)

elif error_type == "504":
    # 6-second delay to trigger upstream 504 Gateway Timeout
    time.sleep(6)

# --- Normal Execution (HTTP 200) ---
print("Content-Type: text/html; charset=utf-8\r\n\r\n")

env_info = [
    ("REQUEST_METHOD", "HTTP method used (GET, POST...)."),
    ("REQUEST_URI", "Original requested URI."),
    ("SCRIPT_NAME", "Path of the CGI script."),
    ("QUERY_STRING", "Everything after '?''."),
    ("CONTENT_TYPE", "Type of submitted data."),
    ("CONTENT_LENGTH", "Size of the request body in bytes."),
    ("SERVER_PROTOCOL", "HTTP version used."),
    ("SERVER_NAME", "Server hostname."),
    ("SERVER_PORT", "Listening port."),
    ("REMOTE_ADDR", "Client IP address."),
]

# ==========================================
# 3. HTML & CSS LAYOUT
# ==========================================
print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Python CGI File Processor</title>
    <style>
        :root {
            --bg-color: #f8fafc;
            --card-bg: #ffffff;
            --text-main: #1e293b;
            --text-muted: #64748b;
            --primary: #0f172a;
            --border: #e2e8f0;
            --bg-code: #f1f5f9;
        }
        body {
            font-family: -apple-system, BlinkMacSystemFont, sans-serif;
            background-color: var(--bg-color);
            color: var(--text-main);
            padding: 2rem 1rem;
        }
        .container { max-width: 900px; margin: 0 auto; }
        .card {
            background: var(--card-bg);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 1.75rem;
            margin-bottom: 2rem;
            box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.05);
        }
        h1 { color: #3776ab; margin-bottom: 1.5rem; display: flex; align-items: center; gap: 0.5rem;}
        h2 { font-size: 1.3rem; margin-bottom: 1rem; border-bottom: 2px solid var(--border); padding-bottom: 0.5rem; }
        table { width: 100%; border-collapse: collapse; margin-top: 0.5rem; font-size: 0.95rem; }
        th, td { padding: 0.85rem 1rem; text-align: left; border-bottom: 1px solid var(--border); }
        th { background-color: #f8fafc; color: var(--text-muted); font-weight: 600; font-size: 0.75rem; text-transform: uppercase; }
        code { font-family: monospace; background-color: var(--bg-code); padding: 0.2rem 0.4rem; border-radius: 4px; color: #e11d48; }
        .badge { display: inline-block; padding: 0.25rem 0.75rem; font-weight: bold; border-radius: 9999px; font-size: 0.85rem; }
        .badge-post { background: #d1fae5; color: #065f46; }
        .badge-get { background: #dbeafe; color: #1e40af; }
        
        /* Custom file response notification alert style */
        .upload-alert {
            background: #f0fdf4;
            border: 1px solid #bbf7d0;
            color: #166534;
            padding: 1rem;
            border-radius: 8px;
            margin-bottom: 1.5rem;
        }
        .upload-error {
            background: #fef2f2;
            border: 1px solid #fecaca;
            color: #991b1b;
            padding: 1rem;
            border-radius: 8px;
            margin-bottom: 1.5rem;
        }
    </style>
</head>
<body>
<div class="container">
    <h1>🐍 Python CGI File Processor</h1>

    <div class="card">
        <h2>CGI Environment Variables</h2>
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

for key, desc in env_info:
    raw_val = os.environ.get(key, "(not set)")
    if key == "REQUEST_METHOD" and raw_val != "(not set)":
        badge_class = f"badge-{raw_val.lower()}" if raw_val in ["GET", "POST"] else ""
        value = f'<span class="badge {badge_class}">{raw_val}</span>'
    else:
        value = f"<code>{html.escape(raw_val)}</code>" if raw_val != "(not set)" else f'<span style="color:var(--text-muted);">{raw_val}</span>'

    print(f"""
                <tr>
                    <td><code>{key}</code></td>
                    <td>{value}</td>
                    <td>{desc}</td>
                </tr>""")

print("""
            </tbody>
        </table>
    </div>

    <div class="card">
        <h2>Processed Form & File Data</h2>
""")

# Process elements, capturing metadata and copying binary file streams to the upload directory
has_payload = False
for key in form.keys():
    has_payload = True
    form_item = form[key]
    
    # Identify binary file buffers inside the CGI packet
    if isinstance(form_item, cgi.FieldStorage) and form_item.filename:
        filename = os.path.basename(form_item.filename)
        save_path = os.path.join(UPLOAD_DIR, filename)
        content_type = form_item.headers.get('Content-Type', 'unknown type')
        
        try:
            # Safe binary block-by-block writing mechanism
            with open(save_path, 'wb') as f_out:
                while True:
                    chunk = form_item.file.read(65536)  # Read in 64KB blocks to optimize memory buffer
                    if not chunk:
                        break
                    f_out.write(chunk)
            
            # File written successfully, calculate final disk size
            saved_size = os.path.getsize(save_path)
            
            print(f"""
            <div class="upload-alert">
                <h3>📁 File Saved Successfully! (Field: <code>{html.escape(key)}</code>)</h3>
                <p style="margin-top: 0.5rem;"><strong>File Name:</strong> {html.escape(filename)}</p>
                <p><strong>MIME Type:</strong> {html.escape(content_type)}</p>
                <p><strong>Final Saved Size:</strong> {saved_size:,} bytes</p>
                <p><strong>Absolute Storage Target:</strong> <code>{html.escape(os.path.abspath(save_path))}</code></p>
            </div>
            """)
        except Exception as write_err:
            print(f"""
            <div class="upload-error">
                <h3>❌ Failed to Save Uploaded File: {html.escape(filename)}</h3>
                <p style="margin-top: 0.5rem;"><strong>Error Details:</strong> {html.escape(str(write_err))}</p>
                <p><em>Tip: Make sure the target directory <code>{html.escape(UPLOAD_DIR)}</code> exists on your server and has active write permissions.</em></p>
            </div>
            """)
    else:
        # Output ordinary text inputs (e.g. name, text inputs, error types)
        val = form.getvalue(key)
        print(f"""
        <p style="margin-bottom: 0.75rem; border-bottom: 1px solid var(--border); padding-bottom: 0.5rem;">
            <strong>Input Name:</strong> <code>{html.escape(key)}</code><br>
            <strong>Captured Value:</strong> <code>{html.escape(str(val))}</code>
        </p>
        """)

if not has_payload:
    print("<p style='color: var(--text-muted); font-style: italic;'>No form values, metadata, or binary files were submitted inside this request envelope.</p>")

print("""
    </div>
</div>
</body>
</html>
""")