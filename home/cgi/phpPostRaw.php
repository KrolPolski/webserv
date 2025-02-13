<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>PHP POST -test</title>
</head>
<body>
	
	<form method="POST">
		<label>Username:</label>
		<input type="text" name="username" id="username">
		<br>
		<label>Message:</label>
		<input type="text" name="message" id="message">
		<br>
		<input type="submit">
	</form>

</body>
</html>

<?php

echo "<br>";

$user = isset($_POST['username']) ? $_POST['username'] : 'no_name';
$message = isset($_POST['message']) ? $_POST['message'] : 'no_message';

echo "A user called $user just sent us the following message: <br>";
echo "$message";

?>