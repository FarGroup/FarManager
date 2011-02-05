#!/usr/bin/perl -w

$dest_dr            = "/var/www/api";

$dest_dr_inet       = $dest_dr."/temp";

print "PREPARING INET PROJECT\n";

print "\n  -- clear INET\n";

system "rm -Rf ".$dest_dr."/*";

print "\n  -- making directories tree\n";

mkdir $dest_dr_inet, 0775;
mkdir $dest_dr."/images", 0775;
mkdir $dest_dr."/styles", 0775;

#mk_inet_lng("ru","rus");
#mk_inet_lng("en","eng");
mk_inet_lng("ru2","rus2");
#mk_inet_lng("en2","eng2");

system "svn export -q --force http://localhost/svn/enc/trunk/tools/inet/ ".$dest_dr_inet."/inet";
system "cp -f ".$dest_dr_inet."/inet/index.html ".$dest_dr;
system "cp -f ".$dest_dr_inet."/inet/farenclogo.gif ".$dest_dr."/images/farenclogo.gif";
system "cp -f ".$dest_dr_inet."/inet/styles.css ".$dest_dr."/styles/styles.css";

system "rm -Rf ".$dest_dr_inet;

system "chown -R :webadmins ".$dest_dr."/*";

sub mk_inet_lng
{
  local($dr1) = $_[0];
  local($dr2) = $_[1];

  print " -- preparing ".$dr1." --\n";
  system "svn export -q --force http://localhost/svn/enc/trunk/enc_".$dr2." ".$dest_dr_inet."/meta.".$dr1;
  mkdir $dest_dr."/".$dr1."/", 0775;
  system "cp -Rf ".$dest_dr_inet."/meta.".$dr1."/meta/* ".$dest_dr."/".$dr1;
  system "cp -Rf ".$dest_dr_inet."/meta.".$dr1."/images/* ".$dest_dr."/images";
  system "rm -Rf ".$dest_dr_inet."/meta.".$dr1;
  $lev = -1;
  fix($dest_dr."/".$dr1);
};

sub fix
{
 $lev++;

 local($dr1) = $_[0];
 local($dr)=$dr1."/";
 printf "$dr\n";

 foreach(`ls -1 $dr1`){
   chomp;
   if (-d $dr.$_) {
     fix("$dr$_");
   }

   if (/\.html$/){
     open F, $dr.$_;
     @FData = <F>;
     close F;

     open F, ">$dr$_";
     foreach(@FData){
       #if (/<meta\s.*?Windows-1251.*$/xi){
       #  next;
       #};
       $back = "../"x$lev;
       s[ <a \s* href=\"win32\/ .*?>(.*?)</a> ][ <a href=\"${back}notfound.html?$1\">$1</a> ]igx;
       print F $_;
     };
     close F;
   };
 };
 $lev--;
};
