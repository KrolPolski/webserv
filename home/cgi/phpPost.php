<?php

$user = isset($_POST['username']) ? $_POST['username'] : 'user';
$message = isset($_POST['message']) ? $_POST['message'] : 'Welcome to our php POST request tester! Please write your name and message below.';

if (isset($_POST['message']))
	$text = 'You sent the following message:';
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
			background-color: #e0f7fa;
			color: #2c6e2d;
			text-align: center;
			padding: 50px;
			margin: 0;
			display: flex;
			flex-direction: column;
			justify-content: center;
			height: 100vh;
		}
		h1 {
			color: #388e3c;
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
			border: 1px solid #388e3c;
		}
		textarea {
			height: 150px;
		}
		button {
			font-size: 1.2em;
			padding: 10px 20px;
			background-color: #388e3c;
			color: white;
			border: none;
			border-radius: 5px;
			cursor: pointer;
		}
		button:hover {
			background-color: #2e7d32;
		}
	</style>
</head>
<body>
	<h1>Hello, <?php echo htmlspecialchars($user); ?>!</h1>
	<p><?php echo htmlspecialchars($text); ?></p>
	<p><em><?php echo nl2br(htmlspecialchars($message)); ?></em></p>

	<form method="POST">
		<label for="username">Your Name:</label><br>
		<input type="text" id="username" name="username" required value="<?php echo htmlspecialchars($user); ?>"><br>
		<label for="message">Your Message:</label><br>
		<textarea id="message" name="message" required><?php echo htmlspecialchars($message); ?></textarea><br>
		<button type="submit">Send Message</button>
	</form>
</body>
</html>
