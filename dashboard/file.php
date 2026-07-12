<?php

header("Content-Type: text/html");
header("X-CGI-Test: Webserv");
header("Cache-Control: no-cache");
header("Pragma: no-cache");

?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>PHP CGI Test</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 40px;
            background: #f4f4f4;
        }

        h1 {
            color: #333;
        }

        .box {
            background: white;
            border: 1px solid #ccc;
            padding: 15px;
            margin-bottom: 25px;
            border-radius: 8px;
        }

        pre {
            background: #eee;
            padding: 10px;
            overflow-x: auto;
        }

        input, textarea {
            width: 300px;
            margin: 5px 0;
        }
    </style>
</head>
<body>

<h1>PHP CGI Test Page</h1>

<div class="box">
<h2>PHP is Working!</h2>

<p>
Current server time:
<strong><?php echo date("Y-m-d H:i:s"); ?></strong>
</p>

<p>
PHP Version:
<strong><?php echo phpversion(); ?></strong>
</p>

</div>

<div class="box">

<h2>Request Information</h2>

<pre>
Method: <?php echo $_SERVER["REQUEST_METHOD"]; ?>

Script: <?php echo $_SERVER["SCRIPT_NAME"]; ?>

Query String: <?php echo $_SERVER["QUERY_STRING"] ?? ""; ?>

Content Type: <?php echo $_SERVER["CONTENT_TYPE"] ?? ""; ?>

Content Length: <?php echo $_SERVER["CONTENT_LENGTH"] ?? ""; ?>

Remote Address: <?php echo $_SERVER["REMOTE_ADDR"] ?? ""; ?>

Server Protocol: <?php echo $_SERVER["SERVER_PROTOCOL"] ?? ""; ?>

Server Software: <?php echo $_SERVER["SERVER_SOFTWARE"] ?? ""; ?>

</pre>

</div>

<div class="box">

<h2>GET Test</h2>

<form method="GET">
    Name:
    <input type="text" name="name">
    <input type="submit" value="Send GET">
</form>

<?php
if (!empty($_GET)) {
    echo "<pre>";
    print_r($_GET);
    echo "</pre>";
}
?>

</div>

<div class="box">

<h2>POST Test</h2>

<form method="POST">

Name:<br>
<input type="text" name="username"><br>

Message:<br>
<textarea name="message"></textarea><br>

<input type="submit" value="Send POST">

</form>

<?php
if (!empty($_POST)) {
    echo "<pre>";
    print_r($_POST);
    echo "</pre>";
}
?>

</div>

<div class="box">

<h2>File Upload Test</h2>

<form method="POST" enctype="multipart/form-data">

<input type="file" name="myfile">

<input type="submit" value="Upload">

</form>

<?php

if (!empty($_FILES)) {
    echo "<pre>";
    print_r($_FILES);
    echo "</pre>";
}

?>

</div>

<div class="box">

<h2>Environment Variables</h2>

<pre>

<?php
foreach ($_SERVER as $key => $value)
{
    echo $key . " = " . $value . "\n";
}
?>

</pre>

</div>

</body>
</html>