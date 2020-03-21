<?php
require __DIR__ . '/MoonDbNoSQL.php';

$dbh = new MoonDbNoSQL('127.0.0.1', 8888, 'test');
var_dump($dbh->updateData('testtable', 1, array(
	'title' => 'abc3',
	'content' => 'hgdfgd3',
	'price' => 30.0,
	'hits' => 6,
)));