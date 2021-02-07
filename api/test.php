<?php
//require __DIR__ . '/MoonDbNoSQL.php';

//$dbh = new MoonDbNoSQL('127.0.0.1', 8888, 'test');
//$dbh->test_send();

//echo bin2hex('¹æ¶¨');

$time_start = microtime(true);
$a = 100.0;
for($i = 0; $i < 10000000; $i ++)
	$a = sin($a * ((double)$i + 1.0));
$time = microtime(true) - $time_start;

echo $a."\n".$time;