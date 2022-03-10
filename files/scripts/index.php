#!/bin/php
<!doctype html>
<html>
	<link href="layout.css" rel="stylesheet" type="text/css">
	<head>
		<title>Index</title>
	</head>
	<body>
		<div>
			<p>
				<?php
					$dir = "./";
					$addr = $_SERVER["REMOTE_HOST"];
					$path = $_SERVER["DOCUMENT_ROOT"];
					print $path;
					echo "<br>";
					$data = scandir($dir, SCANDIR_SORT_ASCENDING, null);
					if ($data != false)
					{
						foreach($data as $files)
						{
							echo "http://";
							echo $addr;
							if (substr($addr, -1) != "/"
								&& substr($path, 0, 1) != "/")
								echo "/";
							echo $path;
							if (substr($path, -1) != "/"
								&& substr($files, 0, 1) != "/")
								echo "/";
							echo $files;
							echo "<br>";
						}
					}
				?>
			</p>
		</div>
	</body>
</html>
