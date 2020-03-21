<?php
$socket = socket_create(AF_INET6, SOCK_STREAM, SOL_TCP);
if(!$socket) {
	echo "Can't open socket\n";
}
//接收套接流的最大超时时间，后面是微秒单位超时时间，设置为零，表示不管它
socket_set_option($socket, SOL_SOCKET, SO_RCVTIMEO, array("sec" => 10, "usec" => 0));
//发送套接流的最大超时时间
socket_set_option($socket, SOL_SOCKET, SO_SNDTIMEO, array("sec" => 10, "usec" => 0));
socket_connect($socket, '::1', 8888);
socket_send($socket, "hello", 5, 0);