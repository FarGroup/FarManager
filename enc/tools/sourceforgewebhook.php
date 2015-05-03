<?php
$headers = apache_request_headers();
$request_hash = $headers['X-Allura-Signature'];

$json_input = file_get_contents('php://input');
$json_hash = "sha1=" . hash_hmac('sha1', $json_input, '!secret!');

if ($request_hash != $json_hash) exit();

$postcommitmsg = json_decode($json_input, true);

if ($postcommitmsg['size'] < 1) exit();

exec("svnsync sync file:///home/alex/.wine/drive_c/src/syncrepo");
?>
