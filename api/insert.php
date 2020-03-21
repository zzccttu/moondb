<?php
require __DIR__ . '/MoonDbNoSQL.php';

$dbh = new MoonDbNoSQL('127.0.0.1', 8888, 'test');
echo $dbh->insertData('testtable', array(
	'title' => 'abc',
	'content' => 'hgdfgd',
	'price' => 10.0,
	'hits' => 2,
));