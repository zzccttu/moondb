<?php
require __DIR__ . '/MoonDbNoSQL.php';

$dbh = new MoonDbNoSQL('127.0.0.1', 8888, 'test');

$start_time = microtime(true);

$rows = 0;
for($i = 0; $i < 50000; $i++) {
	if(is_array($dbh->getData('testtable', $i + 1)))
		$rows++;
}

$end_time = microtime(true);

echo $rows;
echo "\n";
echo $end_time - $start_time;
echo "\n";