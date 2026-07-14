function log(msg, type = "info") {
    const consoleDiv = document.getElementById("log-output");
    const time = new Date().toLocaleTimeString();
    let color = type === "error" ? "#f87171" : type === "success" ? "#34d399" : "#94a3b8";
    
    consoleDiv.innerHTML += `<span style="color: ${color}">[${time}] ${msg}</span>\n`;
    consoleDiv.scrollTop = consoleDiv.scrollHeight;
}

async function testFetch(method, url, body = null, useCredentials = false) {
    log(`>> ${method} ${url}`, "info");
    const options = { method: method, headers: {} };

    if (body) {
        options.body = body;
        options.headers['Content-Type'] = 'text/plain';
    }

    if (useCredentials) {
        options.credentials = 'include'; // Required to send cookies to the server
    }

    try {
        const start = performance.now();
        const response = await fetch(url, options);
        const timeTaken = (performance.now() - start).toFixed(0);
        
        log(`<< ${response.status} ${response.statusText} (${timeTaken}ms)`, response.ok ? "success" : "error");
        
        const text = await response.text();
        log(`Body: ${text.substring(0, 80).replace(/\n/g, ' ')}${text.length > 80 ? '...' : ''}\n`);
    } catch (err) {
        log(`!! Error: ${err.message}\n`, "error");
    }
}

async function testMultipartAjax() {
    const formData = new FormData();
    formData.append("user", "tester");
    formData.append("file", new Blob(["Hello Webserv"], {type: "text/plain"}), "hello.txt");
    
    log(">> POST /cgi-bin/multipart.pl (AJAX Multipart)", "info");
    try {
        const response = await fetch('/cgi-bin/multipart.pl', { method: 'POST', body: formData });
        log(`<< ${response.status} ${response.statusText}`, response.ok ? "success" : "error");
    } catch (err) {
        log(`!! Error: ${err.message}\n`, "error");
    }
}

function sendMassivePayload() {
    log(">> Generating 10MB payload (Testing chunking/limits)...");
    const chunk = "A".repeat(1024 * 1024 * 10); 
    testFetch('POST', '/uploads', chunk);
}

function clearCookies() {
    document.cookie.split(";").forEach(function(c) { 
        document.cookie = c.replace(/^ +/, "").replace(/=.*/, "=;expires=" + new Date().toUTCString() + ";path=/"); 
    });
    log(">> Local browser cookies cleared.", "success");
}

// A self-healing delete test. It ensures the file exists before deleting it.
async function testRobustDelete() {
    log(">> [Phase 1] Creating temporary file for deletion...", "info");
    try {
        // 1. Create the file first
        const createRes = await fetch('/uploads/temp_delete_test.txt', {
            method: 'POST',
            headers: { 'Content-Type': 'text/plain' },
            body: 'This file is meant to be deleted.'
        });
        
        if (!createRes.ok) {
            log(`!! Failed to create file for DELETE test. Status: ${createRes.status}`, "error");
            return;
        }
        log(`<< File created successfully. Waiting 1s...`, "success");
        
        // Wait 1 second so you can see it happen in your server logs
        await new Promise(resolve => setTimeout(resolve, 1000));

        // 2. Now delete it
        log(">> [Phase 2] Sending DELETE request...", "info");
        const deleteRes = await fetch('/uploads/temp_delete_test.txt', {
            method: 'DELETE'
        });

        log(`<< DELETE Response: ${deleteRes.status} ${deleteRes.statusText}`, deleteRes.ok ? "success" : "error");
        
        // 204 No Content or 200 OK are the standard success codes for DELETE
        if (deleteRes.status === 204 || deleteRes.status === 200) {
            log(">> DELETE operation completely successful.", "success");
        }
    } catch (err) {
        log(`!! Robust Delete Error: ${err.message}`, "error");
    }
}