<?php
require __DIR__ . '/MoonDbNoSQL.php';

$dbh = new MoonDbNoSQL('127.0.0.1', 8888, 'test');

$start_time = microtime(true);

$affectedrows = 0;
for($i = 0; $i < 30000; $i++) {
	$affectedrows += $dbh->updateData('testtable', $i + 1, array(
		'title' => 'abc3',
		'content' => 'hgdfgd3',
		'price' => 30.0,
		'hits' => 6,
	));
}

$end_time = microtime(true);

echo $affectedrows;
echo "\n";
echo $end_time - $start_time;
echo "\n";