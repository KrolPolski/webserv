<?php
header('Content-Type: text/html');

// Get the username and hobby from the URL parameters
$user = isset($_GET['username']) ? $_GET['username'] : 'no_name';
$hobby = isset($_GET['hobby']) ? $_GET['hobby'] : 'no_hobby';
?>
<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Hello user -page</title>
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
	</style>
</head>
<body>
	<h1>Hello, <?php echo htmlspecialchars($user); ?>!</h1>
	<p>You must really love <?php echo htmlspecialchars($hobby); ?>!</p>
</body>
</html>
