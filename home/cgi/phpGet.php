<?php
header('Content-Type: text/html');

// Get the username and message from the URL parameters
$user = isset($_GET['username']) ? $_GET['username'] : 'user';
$hobby = isset($_GET['hobby']) ? $_GET['hobby'] : 'Welcome to our php GET request tester! Please write your name and hobby below.';

if (isset($_GET['hobby']))
	$text = 'Your hobby is:';
else 
	$text = '';

?>
<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Message from User</title>
	<style>
		body {
			font-family: Arial, sans-serif;
			background-color: #ffebee; /* Light red background */
			color: #b71c1c; /* Dark red text */
			text-align: center;
			padding: 50px;
			margin: 0;
			display: flex;
			flex-direction: column;
			justify-content: center;
			height: 100vh;
		}
		h1 {
			color: #d32f2f; /* Medium red for heading */
			font-size: 3em;
		}
		p {
			font-size: 1.5em;
			margin-top: 20px;
		}
		form {
			margin-top: 30px;
			padding: 20px;
			background-color: #ffffff;
			border-radius: 10px;
			box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.1);
		}
		input, textarea {
			font-size: 1em;
			padding: 10px;
			margin: 10px 0;
			width: 100%;
			max-width: 400px;
			border-radius: 5px;
			border: 1px solid #d32f2f;
		}
		textarea {
			height: 150px;
		}
		button {
			font-size: 1.2em;
			padding: 10px 20px;
			background-color: #d32f2f;
			color: white;
			border: none;
			border-radius: 5px;
			cursor: pointer;
		}
		button:hover {
			background-color: #c62828;
		}
	</style>
</head>
<body>
	<h1>Hello, <?php echo htmlspecialchars($user); ?>!</h1>
	<p><?php echo htmlspecialchars($text); ?></p>
	<p><em><?php echo nl2br(htmlspecialchars($hobby)); ?></em></p>

	<form method="GET">
		<label for="username">Your Name:</label><br>
		<input type="text" id="username" name="username" required value="<?php echo htmlspecialchars($user); ?>"><br>
		<label for="hobby">Your Hobby:</label><br>
		<textarea id="hobby" name="hobby" required><?php echo htmlspecialchars($hobby); ?></textarea><br>
		<button type="submit">Send Info</button>
	</form>
</body>
</html>

