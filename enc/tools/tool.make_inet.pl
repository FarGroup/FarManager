#!/usr/bin/perl -w

$dest_dr            = "../enc";

$dest_dr_inet       = $dest_dr."/inet";

print "PREPARING INET PROJECT\n";

print "\n  -- clear INET\n";

system "rm -f -r ".$dest_dr_inet;

print "\n  -- making directories tree\n";

mkdir $dest_dr, 0775;
mkdir $dest_dr_inet, 0775;
mkdir $dest_dr_inet."/images", 0775;
mkdir $dest_dr_inet."/styles", 0775;

#mk_inet_lng("ru","rus");
#mk_inet_lng("en","eng");
mk_inet_lng("ru2","rus2");
#mk_inet_lng("en2","eng2");

system "cp -f inet/index.html ".$dest_dr_inet."/index.html";
system "cp -f inet/farenclogo.gif ".$dest_dr_inet."/images/farenclogo.gif";
system "cp -f inet/styles.css ".$dest_dr_inet."/styles/styles.css";

sub mk_inet_lng
{
  local($dr1) = @_[0];
  local($dr2) = @_[1];

  print " -- preparing ".$dr1." --\n";
  system "svn export ../enc_".$dr2." ".$dest_dr_inet."/meta.".$dr1;
  mkdir $dest_dr_inet."/".$dr1."/", 0775;
  system "cp -R -f ".$dest_dr_inet."/meta.".$dr1."/meta/* ".$dest_dr_inet."/".$dr1;
  system "cp -R -f ".$dest_dr_inet."/meta.".$dr1."/images/* ".$dest_dr_inet."/images";
  system "rm -f -r ".$dest_dr_inet."/meta.".$dr1;
  $lev = -1;
  fix($dest_dr_inet."/".$dr1);
};

sub fix
{
 $lev++;

 local($dr1) = @_[0];
 local($dr)=$dr1."/";
 printf "$dr\n";

 foreach(`ls -1 $dr1`){
   chomp;
   if (-d $dr.$_) {
     fix("$dr$_");
   }

   if (/\.html$/){
     $fn = $_;
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
