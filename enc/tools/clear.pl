#!/usr/bin/perl -w

$dest_dr            = "../enc";

$toolpath           = "";
if($^O eq "MSWin32"){
  $toolpath="./tools/";
}

system $toolpath."rm -f -r ".$dest_dr;

print "done\n";
