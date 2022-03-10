#!/bin/php
<!doctype html>
<html>
	<link href="layout.css" rel="stylesheet" type="text/css">
	<head>
		<title>Post Form</title>
	</head>
	<body>
		<p>
<?php
	if ($_POST["things"])
	{
		print "\t\t\t";
		print "things=";
		print $_POST["things"];
		print "\n";
	}
	else
		print "undefined variable things";
?>
		</p>
	</body>
</html>
