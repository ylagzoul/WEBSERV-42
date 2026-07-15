<?php
// 1. Extend the PHP internal script execution limit so PHP itself doesn't crash before the server does
set_time_limit(200);

// 2. Simulate a long-running process by sleeping for 10 seconds.
// (If your web server gateway timeout is set lower than 10s, this will force a 504 error).
sleep(100);

// 3. If the server does not timeout, return a success response
header("Content-Type: text/plain; charset=UTF-8");
echo "SUCCESS: The PHP script finished executing after 10 seconds without timing out.";