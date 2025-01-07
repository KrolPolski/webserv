<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>PHP POST - Test</title>
</head>
<body>

<?php
echo $_SERVER["REQUEST_METHOD"];
echo "<br>";
// Check if the form is submitted via POST
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Ensure both 'username' and 'message' are in the POST data
    if (isset($_POST['username']) && isset($_POST['message'])) {
        // Sanitize input to avoid XSS (Cross-site Scripting)
        $user = htmlspecialchars($_POST['username']);
        $message = htmlspecialchars($_POST['message']);

        // Output the result
        echo "<h2>A user called $user just sent us the following message:</h2>";
        echo "<p>$message</p>";
    } else {
		echo "<p>We are actually inside the post if check</p>";
        // If required fields are missing, display an error message
        echo "<p>Error: Please fill out all fields in the form.</p>";
    }
} else {
    // If form is not submitted, ask user to submit it
    echo "<p>Please submit the form.</p>";
}
?>

</body>
</html>
