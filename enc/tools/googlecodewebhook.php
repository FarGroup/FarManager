<?php
$headers = apache_request_headers();
$request_hash = $headers['Google-Code-Project-Hosting-Hook-HMAC'];

$json_input = file_get_contents('php://input');
$json_hash = hash_hmac('md5', $json_input, '!secret!');

if ($request_hash != $json_hash) exit();

$postcommitmsg = json_decode($json_input, true);

if ($postcommitmsg['revision_count'] < 1) exit();

exec("svnsync sync file:///home/alex/.wine/drive_c/src/fromgoogle");
?>
