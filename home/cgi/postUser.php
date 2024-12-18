<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Post user -page</title>
</head>
<body>
	
<?php

$user = $_POST['username'];
$message = $_POST['message'];

echo "A user called $user just sent us the following message: <br>";
echo "$message";

?>


</body>
</html>