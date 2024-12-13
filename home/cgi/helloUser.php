#! /usr/bin/php

<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Hello user page</title>
</head>
<body>
	
<?php

$user = $_GET["username"];
$hobby = $_GET["hobby"];

echo "Hello user $user! <br>";
echo "You must really love $hobby <br>";

?>


</body>
</html>