<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>PHP POST -test</title>
	<style>
		body{
			text-align: center;
		}
		.text-box {
			background-color: rgb(156, 232, 156);
			color: black;
			font-size: 1.2em;
			width: 50%;
			margin-left: auto;
			margin-right: auto;
			margin-top: 20px; /* Reduced margin-top */
			padding: 12px 8px;
			border: 2px solid black;
			border-radius: 30px;
			transition: background-color 0.3s ease, transform 0.3s ease;
			text-decoration: none;
			position: relative;
		}

		.test-form {
			background-color: rgb(156, 232, 156);
			color: black;
			font-size: 1.2em;
			width: 50%;
			margin-left: auto;
			margin-right: auto;
			margin-top: 50px; /* Keep some space from top */
			padding: 12px 8px;
			border: 2px solid black;
			border-radius: 30px;
			transition: background-color 0.3s ease, transform 0.3s ease;
			text-decoration: none;
			position: relative;
		}

		.return-button {
            background-color: #228b22; /* Forest green button */
            color: white;
            font-size: 1.2em;
            padding: 12px 24px;
            border: 2px solid black; /* Black border around the button */
            border-radius: 30px;
            cursor: pointer;
            transition: background-color 0.3s ease, transform 0.3s ease;
            text-decoration: none;
            position: absolute;
            top: 20px;
            right: 20px;
        }

        .return-button:hover {
            background-color: #006400; /* Darker green on hover */
            transform: scale(1.05);
        }
		.return-CGI-button {
            background-color: #228b22; /* Forest green button */
            color: white;
            font-size: 1.2em;
            padding: 12px 24px;
            border: 2px solid black; /* Black border around the button */
            border-radius: 30px;
            cursor: pointer;
            transition: background-color 0.3s ease, transform 0.3s ease;
            text-decoration: none;
            position: absolute;
            top: 20px;
            left: 20px;
        }

        .return-CGI-button:hover {
            background-color: #006400; /* Darker green on hover */
            transform: scale(1.05);
        }	
	</style>
</head>
<body>
	
	<form method="POST" class="test-form">
		<label>Username:</label>
		<input type="text" name="username" id="username">
		<br>
		<label>Message:</label>
		<input type="text" name="message" id="message">
		<br>
		<input type="submit">
	</form>

	<div class="text-box">
		<?php

		echo "<br>";

		$user = isset($_POST['username']) ? $_POST['username'] : 'no_name';
		$message = isset($_POST['message']) ? $_POST['message'] : 'no_message';

		echo "A user called $user just sent us the following message: <br>";
		echo "$message";

		?>
	</div>

	<a href="/" class="return-button">Return to Home page</a>
	<a href="/cgi/" class="return-CGI-button">Return to CGI page</a>

</body>
</html>

