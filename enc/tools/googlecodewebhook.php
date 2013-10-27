<?php
$headers = apache_request_headers();
$request_hash = $headers['Google-Code-Project-Hosting-Hook-HMAC'];

$json_input = file_get_contents('php://input');
$json_hash = hash_hmac('md5', $json_input, '!secret!');

if ($request_hash != $json_hash) exit();

$postcommitmsg = json_decode($json_input, true);

if ($postcommitmsg['revision_count'] < 1) exit();

$repo = escapeshellarg('http://farmanager.googlecode.com/svn');

for ($i=0; $i<$postcommitmsg['revision_count']; $i++)
{
	$rev = escapeshellarg($postcommitmsg['revisions'][$i]['revision']);
	exec("/var/www/tool.make_web_post_commit.sh $repo $rev");
}
?>
