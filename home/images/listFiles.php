<?php
// Set the directory where the files are located
$directory = 'uploads';  // Assuming the "uploads" folder is in the same directory as this script

// Check if the directory exists
if (!is_dir($directory)) {
    echo json_encode(["error" => "Directory does not exist."]);
    exit;
}

// Get all files from the directory (excluding '.' and '..')
$files = array_diff(scandir($directory), array('.', '..'));

// Return the files in JSON format
echo json_encode(array_values($files));
?>