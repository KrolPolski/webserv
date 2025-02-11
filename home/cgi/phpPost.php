<?php

ob_end_flush();  // Disables output buffering
flush();         // Forces immediate output

$user = $_POST['username'];
$message = $_POST['message'];


echo "Length of message:";
echo "<br>";
echo strlen($message);
echo "<br>";
echo "<br>";



echo "A user called $user just sent us the following message: <br>";
echo "$message";

?>
