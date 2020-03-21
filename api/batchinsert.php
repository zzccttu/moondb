<?php
$time1 = microtime(true);

require __DIR__ . '/MoonDbNoSQL.php';
$dbh = new MoonDbNoSQL('127.0.0.1', 8888, 'test');
//$dbh = new MoonDbNoSQL('::1', 8888, 'test');

$time2 = microtime(true);

$lastinsertid = 0;
for($i = 0; $i < 50000; $i++) {
	//sleep(4);
	$lastinsertid = $dbh->insertData('testtable', array(
		'title' => 'abc'.($i+1),
		//'content' => str_pad('hgdfgd', 10000, "abcd"),
		'content' => 'hgdfgd',
		'price' => 10.0,
		'hits' => 2,
	));
	//echo $i . ' ' . $lastinsertid."\n";
}

$time3 = microtime(true);

echo $lastinsertid;
echo "\n";
echo $time2 - $time1;echo "\n";
echo $time3 - $time2;echo "\n";