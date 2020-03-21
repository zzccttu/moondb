<?php
class MoonDbNoSQL
{
	const FT_BOOL = 1;
	const FT_BIT = 2;
	const FT_INT8 = 3;
	const FT_UINT8 = 4;
	const FT_INT16 = 5;
	const FT_UINT16 = 6;
	const FT_INT32 = 7;
	const FT_UINT32 = 8;
	const FT_INT64 = 9;
	const FT_UINT64 = 10;
	const FT_INT128 = 11;
	const FT_UINT128 = 12;
	const FT_FLOAT = 17;
	const FT_DOUBLE = 18;
	const FT_LONGDOUBLE = 19;
	const FT_STRING = 31;
	const FT_NULL = 65535;

	const OPER_SELECT = 1;
	const OPER_INSERT = 2;
	const OPER_UPDATE = 3;
	const OPER_DELETE = 4;
	const OPER_REPLACE = 5;

	const RT_ERROR = 1;
	const RT_CONNECT = 2;
	const RT_RECONNECT = 3;
	const RT_QUERY = 4;
	const RT_LAST_INSERT_ID = 5;
	const RT_AFFECTED_ROWS = 6;
	const RT_EXECUTE = 7;

	const Bytes_Per_Read = 8192;

	static protected $paramPackMap = array(
		self::FT_INT8 => 'c',
		self::FT_UINT8 => 'C',
		self::FT_INT16 => 's',
		self::FT_UINT16 => 'S',
		self::FT_INT32 => 'l',
		self::FT_UINT32 => 'L',
		self::FT_INT64 => 'q',
		self::FT_UINT64 => 'Q',
		self::FT_FLOAT => 'f',
		self::FT_DOUBLE => 'd',

		/*self::FT_DECREMENT_DOUBLE => 'd',
		self::FT_INCREMENT_DOUBLE => 'd',
		self::FT_DECREMENT_INT => 'q',
		self::FT_INCREMENT_INT => 'q',*/
	);
	
	static protected $numLengths = array(
		self::FT_INT8 => 1,
		self::FT_UINT8 => 1,
		self::FT_INT16 => 2,
		self::FT_UINT16 => 2,
		self::FT_INT32 => 4,
		self::FT_UINT32 => 4,
		self::FT_INT64 => 8,
		self::FT_UINT64 => 8,
		self::FT_INT128 => 16,
		self::FT_UINT128 => 16,
		self::FT_FLOAT => 4,
		self::FT_DOUBLE => 8,
	);

	protected $useSocket;
	protected $IPv6;
	protected $serverIP;
	protected $serverPort;
	protected $clientTimeout;
	protected $socket;
	protected $database;
	protected $autoReconnect;

	function __construct($ip, $port, $dbname = null, $timeout = 5000000, $autoreconn = false)
	{
		$this->useSocket = function_exists('socket_create');
		if($timeout <= 0)
			$timeout = 5000000;
		$this->IPv6 = strpos($ip, ':') !== false;
		$this->serverIP = $ip;
		$this->serverPort = $port;
		$this->clientTimeout = $timeout;
		$this->database = $dbname;
		$this->autoReconnect = $autoreconn;
		$this->connect();
	}

	function __destruct()
	{
		$this->disconnect();
	}

	protected function connect()
	{
		$timeopt = array("sec" => $this->clientTimeout / 1000000, "usec" => $this->clientTimeout % 1000000);
		if($this->useSocket) {
			$socket = socket_create(($this->IPv6 ? AF_INET6 : AF_INET), SOCK_STREAM, SOL_TCP);
			if(!$socket) {
				$this->throwError("Can't created socket");
			}
			//接收套接流的最大超时时间
			socket_set_option($socket, SOL_SOCKET, SO_RCVTIMEO, $timeopt);
			//发送套接流的最大超时时间
			socket_set_option($socket, SOL_SOCKET, SO_SNDTIMEO, $timeopt);
			// 最多5秒连接
			$itries = 5000;
			do {
				if(@socket_connect($socket, $this->serverIP, $this->serverPort)) {
					break;
				}
				usleep(1000);
			} while(--$itries > 0);
			if($itries == 0) {
				$this->throwError("Can't connect to MoonDb({$this->serverIP}:{$this->serverPort})");
			}
		}
		else {
			$socket = stream_socket_client("tcp://{$this->serverIP}:{$this->serverPort}", $errno, $errstr, 5);
			if(!$socket) {
				$this->throwError("Can't connect to MoonDb: $errstr ($errno)");
			}
			stream_set_timeout($socket, $timeopt["sec"], $timeopt["usec"]);
		}
		$this->socket = $socket;
	}

	protected function disconnect()
	{
		if(is_resource($this->socket)) {
			if($this->useSocket)
				socket_close($this->socket);
			else
				fclose($this->socket);
			$this->socket = null;
		}
	}

	protected function reconnect()
	{
		$this->disconnect();
		$this->connect();
	}

	protected function throwError($msg)
	{
		if($this->useSocket && is_resource($this->socket))
			$errid = ':' . socket_last_error($this->socket);
		// 当无法接收数据时会socket_strerror函数导致程序异常退出，所以不使用它
		//throw new Exception($msg . ':' . ($errid > 0 ? @socket_strerror($errid) : 'unknown error') . '.');
		throw new Exception($msg . $errid . '.');
	}

	function UseDatabase($dbname)
	{
		$this->database = $dbname;
	}

	function packUInt128($val)
	{
		$int32_1 = bcmod($val, 4294967296, 0);
		$val = bcdiv($val, 4294967296, 0);
		$int32_2 = bcmod($val, 4294967296, 0);
		$val = bcdiv($val, 4294967296, 0);
		$int32_3 = bcmod($val, 4294967296, 0);
		$int32_4 = bcdiv($val, 4294967296, 0);
		// 整数字节顺序按照从低位到高位排列
		return pack('VVVV', (int)$int32_1, (int)$int32_2, (int)$int32_3, (int)$int32_4);
	}

	function packInt128($val)
	{
		// 计算负数补码
		if(bccomp($val, 0) < 0) {
			$val = bcadd($val, '340282366920938463463374607431768211456', 0);
		}
		return $this->packUInt128($val);
	}

	function unpackUInt128($val)
	{
		$numbers = unpack('V4', $val);
		return bcadd(bcadd(bcadd($numbers[1], bcmul($numbers[2], '4294967296', 0), 0), 
			bcmul($numbers[3], '18446744073709551616', 0), 0), bcmul($numbers[4], '79228162514264337593543950336'), 0);
	}

	function unpackInt128($val)
	{
		$num = $this->unpackUInt128($val);
		if(bccomp($num, '170141183460469231731687303715884105727', 0) <= 0) {
			return $num;
		}
		return bcsub($num, '340282366920938463463374607431768211456', 0);
	}

	function test_send()
	{
		socket_send($this->socket, $this->packInt128('-140282366920008463463374407433338211453'), 16, 0) . "\n";
		socket_recv($this->socket, $content, 16, 0);
		echo $this->unpackInt128($content);
	}

	protected function send($content)
	{
		$n = strlen($content);
		if($this->useSocket) {
			$bytes = socket_send($this->socket, $content, $n, 0);
		}
		else
			$bytes = stream_socket_sendto($this->socket, $content);
		if($n !== $bytes) {
			$this->throwError("An error occor when sending data");
		}
	}

	protected function idNumResult($retcon)
	{
		$idtype = current(unpack('S', substr($retcon, 0, 2)));
		if(!isset(self::$numLengths[$idtype]) || strlen($retcon) != 2 + self::$numLengths[$idtype]) {
			throw new Exception("Wrong length of id or rows is received.");
		}
		$idstr = substr($retcon, 2);
		switch($idtype) {
		case self::FT_INT8:
		case self::FT_UINT8:
		case self::FT_INT16:
		case self::FT_UINT16:
		case self::FT_INT32:
		case self::FT_UINT32:
		case self::FT_INT64:
		case self::FT_UINT64:
			return current(unpack(self::$paramPackMap[$idtype], $idstr));
		case self::FT_INT128:
			return $this->unpackInt128($idstr);
		case self::FT_UINT128:
			return $this->unpackUInt128($idstr);
		default:
			return null;
		}
	}

	// 返回插入值的id
	function insertData($table, $data)
	{
		$datatobesent = $this->prepareData(self::OPER_INSERT, $table, $data);
		$this->send($datatobesent);
		if(!is_array($ret = $this->receiveData($this->autoReconnect))) {
			$this->send($datatobesent);
			$ret = $this->receiveData(false);
		}
		list($rettype, $retcon) = $ret;
		if(self::RT_LAST_INSERT_ID == $rettype) {
			return $this->idNumResult($retcon);
		}
		else {
			return 0;
		}
	}

	// 返回影响的行数
	function updateData($table, $id, $data)
	{
		$data['rowid'] = (string)$id;
		$datatobesent = $this->prepareData(self::OPER_UPDATE, $table, $data);
		$this->send($datatobesent);
		if(!is_array($ret = $this->receiveData($this->autoReconnect))) {
			$this->send($datatobesent);
			$ret = $this->receiveData(false);
		}
		list($rettype, $retcon) = $ret;
		if(self::RT_AFFECTED_ROWS == $rettype) {
			return $this->idNumResult($retcon);
		}
		else {
			return 0;
		}
	}

	// 返回影响的行数
	function deleteData($table, $id)
	{
		$datatobesent = $this->prepareData(self::OPER_DELETE, $table, array('rowid' => (string)$id));
		$this->send($datatobesent);
		if(!is_array($ret = $this->receiveData($this->autoReconnect))) {
			$this->send($datatobesent);
			$ret = $this->receiveData(false);
		}
		list($rettype, $retcon) = $ret;
		if(self::RT_AFFECTED_ROWS == $rettype) {
			return $this->idNumResult($retcon);
		}
		else {
			return 0;
		}
	}

	function getData($table, $id)
	{
		$datatobesent = $this->prepareData(self::OPER_SELECT, $table, array('rowid' => (string)$id));
		$this->send($datatobesent);
		if(!is_array($ret = $this->receiveData($this->autoReconnect))) {
			$this->send($datatobesent);
			$ret = $this->receiveData(false);
		}
		list($rettype, $rawdata) = $ret;

		if(self::RT_QUERY == $rettype) {
			$retdata = array();
			$num = current(unpack('S', substr($rawdata, 0, 2)));
			if($num > 0) {
				$pos = 2;
				$idtype = current(unpack('S', substr($rawdata, $pos, 2)));
				$pos += 2;
				$idlen = self::$numLengths[$idtype];
				$id = current(unpack(self::$paramPackMap[$idtype], substr($rawdata, $pos, $idlen)));
				$pos += $idlen;
				$fieldnum = current(unpack('S', substr($rawdata, $pos, 2)));
				$pos += 2;
				for($i = 0; $i < $fieldnum; $i ++) {
					$fieldnamelen = current(unpack('S', substr($rawdata, $pos, 2)));
					$pos += 2;
					$fieldname = substr($rawdata, $pos, $fieldnamelen);
					$pos += $fieldnamelen;
					$fieldtype = current(unpack('S', substr($rawdata, $pos, 2)));
					$pos += 2;
					switch($fieldtype) {
					case self::FT_INT8:
					case self::FT_UINT8:
					case self::FT_INT16:
					case self::FT_UINT16:
					case self::FT_INT32:
					case self::FT_UINT32:
					case self::FT_INT64:
					case self::FT_UINT64:
					case self::FT_FLOAT:
					case self::FT_DOUBLE:
						$datalen = self::$numLengths[$fieldtype];
						$retdata[$fieldname] = current(unpack(self::$paramPackMap[$fieldtype], substr($rawdata, $pos, $datalen)));
						$pos += $datalen;
						break;
					case self::FT_INT128:
						$retdata[$fieldname] = $this->unpackInt128(substr($rawdata, $pos, 16));
						$pos += 16;
						break;
					case self::FT_UINT128:
						$retdata[$fieldname] = $this->unpackUInt128(substr($rawdata, $pos, 16));
						$pos += 16;
						break;
					case self::FT_BOOL:
						$retdata[$fieldname] = (bool)current(unpack('C', substr($rawdata, $pos, 1)));
						$pos += 1;
						break;
					case self::FT_STRING:
						$datalen = current(unpack('l', substr($rawdata, $pos, 4)));
						$pos += 4;
						$retdata[$fieldname] = substr($rawdata, $pos, $datalen);
						$pos += $datalen;
					}
				}
			}
			return $retdata;
		}
		else {
			return null;
		}
	}

	// 返回影响的行数
	function replaceData($table, $id, $data)
	{
		$data['rowid'] = (string)$id;
		$datatobesent = $this->prepareData(self::OPER_REPLACE, $table, $data);
		$this->send($datatobesent);
		if(!is_array($ret = $this->receiveData($this->autoReconnect))) {
			$this->send($datatobesent);
			$ret = $this->receiveData(false);
		}
		list($rettype, $retcon) = $ret;
		if(self::RT_AFFECTED_ROWS == $rettype) {
			return $this->idNumResult($retcon);
		}
		else {
			return 0;
		}
	}

	protected function prepareData($oper, $table, $data)
	{
		$content = pack('qCS', 0, 1, $oper) . 
				   pack('S', strlen($this->database)) . $this->database .
				   pack('S', strlen($table)) . $table .
				   pack('S', count($data));
		foreach($data as $field => $value) {
			$content .= pack('S', strlen($field)) . $field;
			if(is_scalar($value)) {
				if(is_bool($value)) {
					$type = self::FT_BOOL;
				}
				else if(is_int($value)) {
					$type = self::FT_INT64;
				}
				else if(is_float($value)) {
					$type = self::FT_DOUBLE;
				}
				else if(is_null($value)) {
					$type = self::FT_NULL;
				}
				else if(is_string($value)) {
					$type = self::FT_STRING;
				}
				else {
					throw new Exception("Unrecognizable data: " . var_export($value, true));
				}
			}
			else if(is_array($value)) {
				$type = current($value);
				$value = next($value);
			}
			else {
				throw new Exception("Unrecognizable data: " . var_export($value, true));
			}
			$content .= pack('S', $type);
			if(isset(self::$paramPackMap[$type])) {
				$content .= pack(self::$paramPackMap[$type], $value);
			}
			else {
				switch($type) {
				case self::FT_INT128:
					$content .= $this->packInt128($value);
					break;
				case self::FT_UINT128:
					$content .= $this->packUInt128($value);
					break;
				case self::FT_STRING:
					$content .= pack('l', strlen($value)) . $value;
					break;
				case self::FT_BOOL:
					$content .= pack('C', $value ? 1 : 0);
					break;
				case self::FT_BIT:
					$content .= pack('C', strlen($value)) . $value;
					break;
				case self::FT_NULL:
					break;
				default:
					throw new Exception("Unrecognizable data type: " . $type);
				}
			}
		}

		$lenstr = pack('q', strlen($content) - 8);
		for($i = 0; $i < 8; $i++) {
			$content{$i} = $lenstr{$i};
		}

		return $content;
	}

	protected function receive($length, &$content)
	{
		if($this->useSocket) {
			return socket_recv($this->socket, $content, $length, 0);
		}
		else {
			$content = fread($this->socket, $length);
			return $content === false ? false : strlen($content);
		}
	}

	// 返回数组表示成功，返回false表示需要重新发送数据并接收
	protected function receiveData($reconn)
	{
		// 为了避免发送过大的数据被数据库服务拒绝，停止接收剩余数据，以致只能读取一次数据，所以这里多读些数据包含错误信息。
		// 注：如果接收完数据再返回错误信息则无此问题
		$recvbytes = $this->receive(self::Bytes_Per_Read, $content);
		if(false === $recvbytes) {
			if($reconn) {
				$this->reconnect();
				return false;
			}
			else {
				throw new Exception("Can't connect to the server.");
			}
		}
		if($recvbytes < 11) {
			$this->throwError("An error occor when recieving data.");
		}
		$length = current(unpack('q', substr($content, 0, 8)));
		$rettype = current(unpack('S', substr($content, 8, 2)));
		if($rettype < 1 || $rettype > 6) {
			throw new Exception("Wrong returning data type: " . var_export($rettype, true));
		}
		$data = substr($content, 10);
		if(self::RT_ERROR == $rettype) {
			throw new Exception($data);
		}
		// 读取剩余数据
		$read_len = $recvbytes - 8;
		$left_len = $length - $read_len;
		if($left_len < 0) {
			throw new Exception("Extra data has been read: " . var_export($data, true));
		}
		else if($left_len > 0) {
			while(1) {
				$cur_len = min(self::Bytes_Per_Read, $length - $read_len);
				$recv_len = $this->receive($cur_len, $content);
				if($recv_len > 0) {
					$read_len += $recv_len;
					$data .= $content;
					if($length == $read_len) {
						break;
					}
				}
				else if(false === $recv_len) {
					$this->throwError("An error occor when recieving data");
				}
			}
		}
		return array($rettype, $data);
	}
}