<?php
/*require __DIR__ . '/MoonDbNoSQL.php';
$lastinsertid = 0;
for($i = 0; $i < 5000; $i++) {
	$dbh = new MoonDbNoSQL('127.0.0.1', 8888, 'test');
	$lastinsertid = $dbh->insertData('testtable', array(
		'title' => 'abc',
		'content' => 'hgdfgd',
		'price' => 10.0,
		'hits' => 2,
	));
	$dbh->__destruct();
}
echo $lastinsertid."\n";*/


/*$fp = fsockopen("127.0.0.1", 9999, $errno, $errstr, 30);
fwrite($fp, "test2 data");
$content = fread($fp, 8192) . "\n";
//echo $content;
fclose($fp);*/

//$connection = new PDO("mysql:dbname=test;host=127.0.0.1", 'root', '123456');
//$connection->query('SELECT * FROM test');
//echo $connection->quote('"\'abc')."\n";

//$mysqli = new mysqli('127.0.0.1', 'root', '123456', 'test');
//$mysqli->close();

$time1 = microtime(true);
$dbh = new PDO("mysql:dbname=test;host=127.0.0.1", 'root', '123456');
//sleep(10);
$time2 = microtime(true);
$sql = 'UPDATE test SET title=?,content=?,price=?,hits=? WHERE id=?';
$sth = $dbh->prepare($sql);
for($i = 0; $i < 30000; $i++) {
	$sth->execute(array('abc3', 'hgdfgd3', 30.0, 6, $i + 1));
}


$time3 = microtime(true);
echo $time2 - $time1;echo "\n";
echo $time3 - $time2;echo "\n";