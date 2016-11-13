<?php

//This script has been used to create the *_script_loader.cpp files in scripts project

$current_dir = basename(getcwd());

function getDirContents($dir, &$results = array() ){
    $files = scandir($dir);

    foreach($files as $key => $value){
        $path = realpath($dir.DIRECTORY_SEPARATOR.$value);
        if(!is_dir($path)) {
            $results[] = $path;
        } else if($value != "." && $value != "..") {
            getDirContents($path, $results);
            $results[] = $path;
        }
    }

    return $results;
}

function extract_addSC(&$function_list, $filename) {
	$content = file_get_contents($filename);
	$matches = false;
	$pattern = "/void (AddSC.+)\(\)/i";
	if(preg_match($pattern, $content, $matches))
		array_push($function_list, $matches[1]);
}

function endsWith($haystack, $needle)
{
    $length = strlen($needle);
    if ($length == 0) {
        return true;
    }

    return (substr($haystack, -$length) === $needle);
}

$files = getDirContents('.');
$function_list = array();

foreach($files as $key => $value) {
	if(endsWith($value, ".cpp"))
		extract_addSC($function_list, $value);
}

foreach($function_list as $key => $value) {
	echo "void $value();" . PHP_EOL;
	
}

echo PHP_EOL . "void Add" . $current_dir . "Scripts()" . PHP_EOL;
echo "{" . PHP_EOL;
foreach($function_list as $key => $value) {
	echo "    $value();" . PHP_EOL;
	
}
echo "}" . PHP_EOL;