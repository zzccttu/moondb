<?php
require __DIR__ . '/MoonDbNoSQL.php';

$dbh = new MoonDbNoSQL('127.0.0.1', 8888, 'test');
$dbh->test_send();
//echo getmypid();
