<?php
// 1. Start output buffering immediately so automatic PHP limits/warnings do not block header changes
ob_start();

ini_set('display_errors', 1);
error_reporting(E_ALL);

// ==========================================
// CONFIGURATION & DIRECTORY SETUP
// ==========================================
$UPLOAD_DIR = "./uploads";

// Ensure the directory exists
if (!is_dir($UPLOAD_DIR)) {
    mkdir($UPLOAD_DIR, 0777, true);
}

// 2. Process simulated error flags
$error_type = $_POST['simulate_error'] ?? $_GET['simulate_error'] ?? 'normal';

if ($error_type === '502') {
    header("Status: Faulty Header Stream");
    echo "Corrupt header generated for testing";
    ob_end_flush();
    exit(1);
}

if ($error_type === '504') {
    sleep(6);
}

header("Content-Type: text/html; charset=UTF-8");

// Prepare Environment Variables data
$env_vars = [
    "REQUEST_METHOD" => ["HTTP Action", $_SERVER["REQUEST_METHOD"] ?? "(not set)"],
    "REQUEST_URI"    => ["Requested Path", $_SERVER["REQUEST_URI"] ?? "(not set)"],
    "SCRIPT_NAME"    => ["Script Entry", $_SERVER["SCRIPT_NAME"] ?? "(not set)"],
    "QUERY_STRING"   => ["GET Parameters", $_SERVER["QUERY_STRING"] ?? "(not set)"],
    "CONTENT_TYPE"   => ["Payload Type", $_SERVER["CONTENT_TYPE"] ?? "(not set)"],
    "CONTENT_LENGTH" => ["Payload Size (B)", $_SERVER["CONTENT_LENGTH"] ?? "(not set)"],
    "SERVER_PROTOCOL"=> ["Protocol Version", $_SERVER["SERVER_PROTOCOL"] ?? "(not set)"],
    "SERVER_NAME"    => ["Server Host", $_SERVER["SERVER_NAME"] ?? "(not set)"],
    "SERVER_PORT"    => ["Active Port", $_SERVER["SERVER_PORT"] ?? "(not set)"],
    "REMOTE_ADDR"    => ["Client Address", $_SERVER["REMOTE_ADDR"] ?? "(not set)"]
];
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Console // PHP CGI Inspector</title>
    <style>
        :root {
            --bg-main: #0b0f19;
            --bg-card: #111827;
            --bg-terminal: #1e293b;
            --border-color: #334155;
            --text-primary: #f8fafc;
            --text-secondary: #94a3b8;
            --accent-neon: #a855f7;
            --accent-cyan: #06b6d4;
            --accent-green: #10b981;
            --accent-red: #ef4444;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: 'Courier New', Courier, monospace;
        }

        body {
            background-color: var(--bg-main);
            color: var(--text-primary);
            padding: 2.5rem 1.5rem;
            line-height: 1.6;
        }

        .wrapper { max-width: 1000px; margin: 0 auto; }

        header {
            margin-bottom: 2rem;
            border-bottom: 1px dashed var(--border-color);
            padding-bottom: 1.5rem;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 1rem;
        }

        .title-area h1 { font-size: 1.8rem; color: var(--accent-cyan); font-weight: bold; }
        .title-area p { color: var(--text-secondary); font-size: 0.9rem; margin-top: 0.25rem; }
        .sys-status {
            background: rgba(16, 185, 129, 0.1);
            border: 1px solid var(--accent-green);
            color: var(--accent-green);
            padding: 0.5rem 1rem;
            font-size: 0.8rem;
            text-transform: uppercase;
            border-radius: 4px;
        }

        .grid-layout {
            display: grid;
            grid-template-columns: 1.3fr 1fr;
            gap: 1.5rem;
        }

        @media (max-width: 900px) { .grid-layout { grid-template-columns: 1fr; } }

        .terminal-card {
            background-color: var(--bg-card);
            border: 1px solid var(--border-color);
            border-radius: 8px;
            overflow: hidden;
            box-shadow: 0 10px 15px -3px rgba(0, 0, 0, 0.4);
        }

        .terminal-header {
            background: #1f2937;
            padding: 0.75rem 1rem;
            display: flex;
            align-items: center;
            justify-content: space-between;
            border-bottom: 1px solid var(--border-color);
        }

        .window-dots { display: flex; gap: 6px; }
        .dot { width: 10px; height: 10px; border-radius: 50%; }
        .dot-red { background: #ef4444; }
        .dot-yellow { background: #f59e0b; }
        .dot-green { background: #10b981; }
        .window-title { color: var(--text-secondary); font-size: 0.8rem; }

        .terminal-body { padding: 1.5rem; }

        table { width: 100%; border-collapse: collapse; font-size: 0.85rem; }
        th, td { text-align: left; padding: 0.75rem 0.5rem; border-bottom: 1px solid rgba(51, 65, 85, 0.5); }
        th { color: var(--accent-neon); text-transform: uppercase; font-size: 0.8rem; padding-bottom: 0.5rem; }

        .var-name { color: var(--accent-cyan); font-weight: bold; }
        .var-val { word-break: break-all; color: #e2e8f0; }

        .badge-method { padding: 0.15rem 0.4rem; border-radius: 3px; font-weight: bold; font-size: 0.75rem; }
        .GET { background: #0369a1; color: #e0f2fe; }
        .POST { background: #065f46; color: #d1fae5; }

        .payload-block {
            background-color: var(--bg-terminal);
            border: 1px solid var(--border-color);
            padding: 1rem;
            border-radius: 4px;
            overflow-x: auto;
            margin-top: 0.5rem;
        }

        .payload-line {
            display: flex;
            justify-content: space-between;
            font-size: 0.85rem;
            margin-bottom: 0.5rem;
            border-bottom: 1px dashed rgba(255,255,255,0.05);
            padding-bottom: 0.25rem;
        }

        .payload-line span:first-child { color: var(--accent-cyan); }

        .file-box {
            border: 1px solid var(--accent-green);
            background: rgba(16, 185, 129, 0.05);
            border-radius: 4px;
            padding: 1rem;
            margin-bottom: 1rem;
            font-size: 0.85rem;
        }

        .file-box-err {
            border: 1px solid var(--accent-red);
            background: rgba(239, 68, 68, 0.05);
            border-radius: 4px;
            padding: 1rem;
            margin-bottom: 1rem;
            font-size: 0.85rem;
        }

        .file-box h3 { color: var(--accent-green); font-size: 0.9rem; margin-bottom: 0.5rem; }
        .file-box-err h3 { color: var(--accent-red); font-size: 0.9rem; margin-bottom: 0.5rem; }

        .no-data { color: var(--text-secondary); font-style: italic; font-size: 0.85rem; text-align: center; padding: 1.5rem 0; }
    </style>
</head>
<body>

<div class="wrapper">

    <header>
        <div class="title-area">
            <h1>$ php-cgi-inspector --live</h1>
            <p>Executed via PHP interpreter. Saving uploads to disk and tracing memory buffers.</p>
        </div>
        <div class="sys-status">STATUS: ONLINE</div>
    </header>

    <div class="grid-layout">
        
        <div class="terminal-card">
            <div class="terminal-header">
                <div class="window-dots">
                    <span class="dot dot-red"></span>
                    <span class="dot dot-yellow"></span>
                    <span class="dot dot-green"></span>
                </div>
                <span class="window-title">environment_variables.sh</span>
                <span></span>
            </div>
            
            <div class="terminal-body">
                <table>
                    <thead>
                        <tr>
                            <th style="width: 35%;">Variable</th>
                            <th style="width: 65%;">Value</th>
                        </tr>
                    </thead>
                    <tbody>
                        <?php foreach ($env_vars as $key => $data): ?>
                            <tr>
                                <td class="var-name"><?php echo htmlspecialchars($key); ?></td>
                                <td class="var-val">
                                    <?php if ($key === "REQUEST_METHOD" && $data[1] !== "(not set)"): ?>
                                        <span class="badge-method <?php echo strtr(htmlspecialchars($data[1]), [" "=>""]); ?>">
                                            <?php echo htmlspecialchars($data[1]); ?>
                                        </span>
                                    <?php elseif ($data[1] !== "(not set)"): ?>
                                        <?php echo htmlspecialchars($data[1]); ?>
                                    <?php else: ?>
                                        <span style="color: var(--text-secondary); font-style: italic;"><?php echo htmlspecialchars($data[1]); ?></span>
                                    <?php endif; ?>
                                </td>
                            </tr>
                        <?php endforeach; ?>
                    </tbody>
                </table>
            </div>
        </div>

        <div class="terminal-card">
            <div class="terminal-header">
                <div class="window-dots">
                    <span class="dot dot-red"></span>
                    <span class="dot dot-yellow"></span>
                    <span class="dot dot-green"></span>
                </div>
                <span class="window-title">stdin_payload_parser.php</span>
                <span></span>
            </div>

            <div class="terminal-body">
                <h3 style="color: var(--accent-cyan); font-size: 0.95rem; margin-bottom: 1rem;">// Input Key/Value Buffers</h3>
                
                <?php
                $has_payload = false;
                $params = array_merge($_GET, $_POST);

                if (!empty($params)) {
                    $has_payload = true;
                    echo '<div class="payload-block">';
                    foreach ($params as $key => $val) {
                        $display_val = is_array($val) ? implode(', ', $val) : $val;
                        echo '<div class="payload-line">';
                        echo '<span>' . htmlspecialchars($key) . ':</span>';
                        echo '<span>' . htmlspecialchars($display_val) . '</span>';
                        echo '</div>';
                    }
                    echo '</div>';
                }
                ?>

                <h3 style="color: var(--accent-neon); font-size: 0.95rem; margin-top: 1.5rem; margin-bottom: 1rem;">// Binary Multipart Buffers</h3>
                
                <?php
                if (!empty($_FILES)) {
                    foreach ($_FILES as $key => $file) {
                        if ($file['error'] === UPLOAD_ERR_OK) {
                            $has_payload = true;
                            
                            $filename = basename($file['name']);
                            $target_path = $UPLOAD_DIR . '/' . $filename;
                            
                            // Safely move the temporary uploaded file to our permanent uploads folder
                            if (move_uploaded_file($file['tmp_name'], $target_path)) {
                                echo '
                                <div class="file-box">
                                    <h3>📂 File Saved: ' . htmlspecialchars($key) . '</h3>
                                    <div class="payload-line"><span>Name:</span><span>' . htmlspecialchars($filename) . '</span></div>
                                    <div class="payload-line"><span>MIME Type:</span><span>' . htmlspecialchars($file['type']) . '</span></div>
                                    <div class="payload-line"><span>Saved To:</span><span>' . htmlspecialchars($target_path) . '</span></div>
                                    <div class="payload-line"><span>Size:</span><span>' . number_format($file['size']) . ' Bytes</span></div>
                                </div>';
                            } else {
                                echo '
                                <div class="file-box-err">
                                    <h3>❌ Write Error: ' . htmlspecialchars($key) . '</h3>
                                    <p>Failed to move file from temp storage to ' . htmlspecialchars($target_path) . '. Check directory permissions.</p>
                                </div>';
                            }
                        } elseif ($file['error'] !== UPLOAD_ERR_NO_FILE) {
                            $has_payload = true;
                            echo '
                            <div class="file-box-err">
                                <h3>❌ Upload Error code: ' . $file['error'] . '</h3>
                                <p>File was rejected by PHP limits (check post_max_size / upload_max_filesize in php.ini).</p>
                            </div>';
                        }
                    }
                }

                if (!$has_payload) {
                    echo '<p class="no-data">[No standard payload or multipart data detected]</p>';
                }
                ?>
            </div>
        </div>

    </div>
</div>

</body>
</html>
<?php
// Flush output buffer and send everything to browser
ob_end_flush();
?>