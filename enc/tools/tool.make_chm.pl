#!/usr/bin/perl -w

$dest_dr            = "../enc";
$dest_dr_chm        = $dest_dr."/chm";

$toolpath           = "";
if($^O eq "MSWin32"){
  $toolpath="./tools/";
}

# про макросы в конец?
$macrolast          = 0;

print "PREPARING CHM PROJECT\n";

print "\n  -- clear CHM\n";
system $toolpath."rm -f -r ".$dest_dr_chm;

print "  -- making directories tree.\n\n";
mkdir $dest_dr, 0775;
mkdir $dest_dr_chm, 0775;

#%hrefs = ();
#%hrefs2 = ();
#mk_chm_lng("ru","rus","r");
#%hrefs = ();
#%hrefs2 = ();
#mk_chm_lng("en","eng","e");

%hrefs = ();
%hrefs2 = ();
mk_chm_lng("ru2","rus2","r");
#%hrefs = ();
#%hrefs2 = ();
#mk_chm_lng("en2","eng2","e");


sub mk_chm_lng
{
  local($dr1) = @_[0];
  local($dr2) = @_[1];
  local($dr3) = @_[2];

  print "\n------------------------------------\nPREPARING ".$dr1." --\n";
  system "svn export ../enc_".$dr2." ".$dest_dr_chm."/".$dr1;
  mkdir $dest_dr_chm."/".$dr1."/html", 0775;
  mkdir $dest_dr_chm."/".$dr1."/distr_chm_plugins".$dr3, 0775;
  mktree($dest_dr_chm."/".$dr1."/meta");

  print "\n\n  -- translating meta into html\n\n";
  $id = 1;
  fix($dest_dr_chm."/".$dr1."/meta");
  print "total $id win32 links...\n";

  system $toolpath."rm -f -r ".$dest_dr_chm."/".$dr1."/meta";

  $fname=$dest_dr_chm."/".$dr1."/plugins".$dr3.".hhk";
  $rel = "html";
  $dir = $dest_dr_chm."/".$dr1."/".$rel;

  print "\n  -- making chm indexes --\n\n";
  srch($dir);

  open OUT, ">$fname";
  print  OUT <<xx;
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<meta name="GENERATOR" content="Microsoft&reg; HTML Help Workshop 4.1">
<!-- Sitemap 1.0 -->
</HEAD><BODY>
<UL>
xx

foreach $key (sort { lc($a) cmp lc($b) } keys %hrefs){
  print OUT <<xx;
  <LI> <OBJECT type="text/sitemap">
    <param name="Name" value="$key">
    <param name="Local" value="$hrefs{$key}">
    </OBJECT>
xx
};

foreach $key (sort { lc($a) cmp lc($b) } keys %hrefs2){
  if ($hrefs{$key})
  {
    print OUT <<xx;
  <LI> <OBJECT type="text/sitemap">
    <param name="Name" value="$key (Macros)">
    <param name="Local" value="$hrefs2{$key}">
    </OBJECT>
xx
  }
  else
  {
    print OUT <<xx;
  <LI> <OBJECT type="text/sitemap">
    <param name="Name" value="$key">
    <param name="Local" value="$hrefs2{$key}">
    </OBJECT>
xx
  }
};

  print OUT <<xx;
</UL>
</BODY></HTML>
xx
  close OUT;
};


#####################################################################
# Генерит индексный файл по <h1> и <a name="">..</a>
sub srch
{
 local($dr1) = @_[0];
 local($dr) = $dr1."/";
 printf "\n$dr  ";
 $ls=$toolpath."ls --ignore=articles ".$dr1;
 foreach $file (`$ls`){
   chomp $file;
   if (-d $dr.$file){
     srch("$dr$file");
   }
   $macro = 0;
   if ($dr =~ /\/macro\//)
   {
     $macro = 1;
   }
   if ($file =~ /\.html$/ && $file !~ /faq\.html/ && $file !~ /notfound\.html/){
     $fn = $dr.$file;
     open F, $fn;
     @FData = <F>;
     close F;
     $fn =~ /($rel.+?)$/;
     $fn = $1;
     print ".";
     foreach(@FData){
       if (/<h1>(.+?)<\/h1>/i){
         $x1 = $1;
         $x1 =~ s/(&quot;)|[\/\'\"<>]//g;
#         $x1 =~ /(.{1,50})/;
#         $x1 = $1;
#         if (length($x1)==50){
#           $x1 =~ /^(.*)\s([a-zA-ZА-Яа-я]+)$/;
#           $x1 = $1."...";
#         };
         if ($macrolast==0 || $macro==0)
         {
           $hrefs{$x1} = $fn;
         }
         else
         {
           $hrefs2{$x1} = $fn;
         }
       };
       if (/<a.+?name=[\"\'](.+?)[\"\'].*?>(.+?)<\/a>/i){
         $x2 = $1;
         $x1 = $2;
         $x1 =~ s/(&quot;)|[\/\'\"<>]//g;
#         $x1 =~ /(.{1,50})/;
#         $x1 = $1;
#         if (length($x1)==50){
#           $x1 =~ /^(.*)\s([a-zA-ZА-Яа-я]+)$/;
#           $x1 = $1."...";
#         };
         if ($macrolast==0 || $macro==0)
         {
           $hrefs{$x1} = $fn."#$x2";
         }
         else
         {
           $hrefs2{$x1} = $fn."#$x2";
         }
       };
     };
   };
 };
};



sub fix
{
 local($dr1) = @_[0];
 local($dr)=$dr1."/";
 printf "$dr\n";
 $ls=$toolpath."ls -1 ".$dr1;
 foreach(`$ls`){
   chomp;
   if (-d $dr.$_) {
     fix("$dr$_");
   }

   $fn = $_;
   open F, $dr.$_;
   @FData = <F>;
   close F;

   $newdr = $dr;
   $newdr =~ s/\/meta\//\/html\//;
   open F, ">$newdr$_";
   foreach(@FData){
     while(s/href[\s"'=\/\.]*?win32\/([^"']*?)(\.html)?['"].*?>(.*?<\/a>)/
href=JavaScript:link$id.Click()>$3
<object id=link$id type="application\/x-oleobject" classid="clsid:adb880a6-d8ff-11cf-9377-00aa003b7a11">
<param name="Command" value="KLink">
<param name="DefaultTopic" value="">
<param name="Item1" value="">
<param name="Item2" value="$1">
<\/object>
/i){
     $id++;
}
     print F $_;
   };
   close F;
 };
};



sub mktree
{
 local($dr1) = @_[0];
 local($dr) = $dr1."/";

 printf "$dr\n";
 $ls=$toolpath."ls -1 ".$dr1;
 foreach(`$ls`){
   chomp;
   if (-d $dr.$_){
     $dhtml = $dr;
     $dhtml =~ s/\/meta\//\/html\//;
     mkdir($dhtml.$_, 0775) if (!-d $dhtml.$_);

     mktree("$dr$_");
   };
 };
};
