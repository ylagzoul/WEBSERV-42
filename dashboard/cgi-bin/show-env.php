<?php
// Enable error display
ini_set('display_errors', 1);
error_reporting(E_ALL);

// 1. Check for simulated errors
$error_type = $_POST['simulate_error'] ?? 'normal';

if ($error_type === '502') {
    // 502 Bad Gateway: Sends invalid, non-spec headers back to Nginx/Apache
    header("Status: Status Line Corrupt");
    echo "This corrupts CGI output header parser";
    exit(1);
}

if ($error_type === '504') {
    // 504 Gateway Timeout: Freeze the PHP interpreter to wait out web server limits
    sleep(6); 
}

// --- Normal execution ---
header("Content-Type: text/html; charset=UTF-8");
?>
<!DOCTYPE html>
<html>
<head>
    <title>PHP CGI Response</title>
    <style>
        body { font-family: monospace; padding: 20px; background: #fafafa; color: #333; }
        .box { border: 1px solid #ccc; background: #fff; padding: 15px; margin-bottom: 20px; border-radius: 5px; }
        h2 { border-bottom: 1px solid #ddd; padding-bottom: 5px; }
    </style>
</head>
<body>
    <div class="box">
        <h2>🐘 PHP CGI Processed Output</h2>
        
        <p><strong>Name:</strong> <?php echo htmlspecialchars($_POST['name'] ?? 'Not Provided'); ?></p>

        <?php if (isset($_FILES['media_file']) && $_FILES['media_file']['error'] == UPLOAD_ERR_OK): ?>
            <?php 
                $file = $_FILES['media_file'];
                $filename = basename($file['name']);
                $file_size = $file['size'];
                $mime_type = $file['type'];
            ?>
            <div style="background: #f0fff4; padding: 10px; margin-top: 10px; border: 1px solid #c6f6d5; border-radius:4px;">
                <p><strong>Uploaded Filename:</strong> <?php echo htmlspecialchars($filename); ?></p>
                <p><strong>Mime-Type:</strong> <?php echo htmlspecialchars($mime_type); ?></p>
                <p><strong>File Size:</strong> <?php echo $file_size; ?> bytes</p>
            </div>
            <?php if (strpos($mime_type, 'video') !== false): ?>
                <p>🎥 Video file detected.</p>
            <?php endif; ?>
        <?php else: ?>
            <p><em>No file uploaded or file exceeded server upload limits.</em></p>
        <?php endif; ?>

    </div>
</body>
</html>