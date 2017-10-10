<?php

// find /d/sunstrider/sources/src/server/scripts/Outland/hellfire_citadel/ -type f -exec php /d/sunstrider/converter/convert.php {} \;

error_reporting(E_ALL);
require_once(__DIR__ . "/ScriptConverter.php");

if($argc != 2) {
	echo "Wrong arg count " . $argc . PHP_EOL;
	exit(1);
}

$filepath = $argv[1];
if(!file_exists($filepath)) {
	echo "File with path $filepath does not exists or is not readable" . PHP_EOL;
	exit(1);
}

$converter = new ScriptConverter($filepath);
$new_content = $converter->Convert($filepath);