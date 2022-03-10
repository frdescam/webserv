#!/bin/php
<?php
	print "<!doctype html>\n";
	print "<html>\n";
	print "\t<head>\n";
	print "\t\t<title>Variables</title>\n";
	print "\t</head>\n";
	print "\t<body>\n";
	print "\t\t<p>\n";
	if ($_GET["a"] && $_GET["b"])
	{
		print "\t\t\t";
		print "a=";
		print $_GET["a"];
		print "\n";
		print "\t\t\t";
		print "b=";
		print $_GET["b"];
		print "\n";
	}
	else
		print "undefined variables a and/or b";
	print "\t\t</p>\n";
	print "\t</body>\n";
	print "</html>\n"
?>
