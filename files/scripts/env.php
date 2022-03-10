#!/bin/php
<!doctype html>
<html>
	<link href="layout.css" rel="stylesheet" type="text/css">
	<head>
		<title>Env</title>
	</head>
	<body>
		<div id ="Page">
			<p>
	<?php
		print "SERVER_SOFTWARE: ";
		print $_SERVER["SERVER_SOFTWARE"];
		print "<br>";
		print "SERVER_NAME: ";
		print $_SERVER["SERVER_NAME"];
		print "<br>";
		print "GATEWAY_INTERFACE: ";
		print $_SERVER["GATEWAY_INTERFACE"];
		print "<br>";
		print "SERVER_PROTOCOL: ";
		print $_SERVER["SERVER_PROTOCOL"];
		print "<br>";
		print "SERVER_PORT: ";
		print $_SERVER["SERVER_PORT"];
		print "<br>";
		print "REQUEST_METHOD: ";
		print $_SERVER["REQUEST_METHOD"];
		print "<br>";
		print "PATH_INFO: ";
		print $_SERVER["PATH_INFO"];
		print "<br>";
		print "PATH_TRANSLATED: ";
		print $_SERVER["PATH_TRANSLATED"];
		print "<br>";
		print "SCRIPT_NAME: ";
		print $_SERVER["SCRIPT_NAME"];
		print "<br>";
		print "QUERY_STRING: ";
		print $_SERVER["QUERY_STRING"];
		print "<br>";
		print "REMOTE_HOST: ";
		print $_SERVER["REMOTE_HOST"];
		print "<br>";
//		print "REMOTE_ADDR: ";
//		print $_SERVER["REMOTE_ADDR"];
//		print "<br>";
		print "AUTH_TYPE: ";
		print $_SERVER["AUTH_TYPE"];
		print "<br>";
		print "REMOTE_USER: ";
		print $_SERVER["REMOTE_USER"];
		print "<br>";
		print "REMOTE_IDENT: ";
		print $_SERVER["REMOTE_IDENT"];
		print "<br>";
		print "CONTENT_TYPE: ";
		print $_SERVER["CONTENT_TYPE"];
		print "<br>";
		print "CONTENT_LENGTH: ";
		print $_SERVER["CONTENT_LENGTH"];
		print "<br>";
		print "HTTP_ACCEPT: ";
		print $_SERVER["HTTP_ACCEPT"];
		print "<br>";
		print "HTTP_ACCEPT_LANGUAGE: ";
		print $_SERVER["HTTP_ACCEPT_LANGUAGE"];
		print "<br>";
		print "HTTP_USER_AGENT: ";
		print $_SERVER["HTTP_USER_AGENT"];
		print "<br>";
//		print "HTTP_COOKIE: ";
//		print $_SERVER["HTTP_COOKIE"];
//		print "<br>";
		print "HTTP_REFERER: ";
		print $_SERVER["HTTP_REFERER"];
		print "<br>";
	?>
			</p>
		</div>
	</body>
</html>
