<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Post user -page</title>
</head>
<body>
	
<?php

//echo file_get_contents('php://input');
//echo "<br>";
//var_dump($_POST);
//echo "<br>";
//echo ini_get('enable_post_data_reading');

//echo "<br>";
//echo "<br>";

phpinfo();

//echo "<br>";
//echo "<br>";



$user = $_POST['username'];
$message = $_POST['message'];

echo "A user called $user just sent us the following message: <br>";
echo "$message";

?>


</body>
</html>