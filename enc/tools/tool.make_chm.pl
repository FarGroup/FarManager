$dest_dr            = "../../enc";
$dest_dr_inet       = $dest_dr."/inet";
$dest_dr_inet_ru    = $dest_dr_inet."/ru";
$dest_dr_inet_en    = $dest_dr_inet."/en";
$dest_dr_chm        = $dest_dr."/chm";
$dest_dr_chm_ru     = $dest_dr_chm."/ru";
$dest_dr_chm_en     = $dest_dr_chm."/en";

$meta_ru            = $dest_dr_chm_ru."/meta";
$meta_en            = $dest_dr_chm_en."/meta";

print "PREPARING CHM PROJECT\n";

print "\n  -- clear CHM\n";
system "rm -f -r ".$dest_dr_chm;

print "  -- making directories tree.\n\n";
mkdir $dest_dr, 0;
mkdir $dest_dr_chm, 0;

%hrefs = ();
%hrefs2 = ();
mk_chm_lng("ru","rus","r");
%hrefs = ();
%hrefs2 = ();
mk_chm_lng("en","eng","e");


sub mk_chm_lng
{
  local($dr1) = @_[0];
  local($dr2) = @_[1];
  local($dr3) = @_[2];

  print "\n------------------------------------\nPREPARING ".$dr1." --\n";
  system "svn export ../enc_".$dr2." ".$dest_dr_chm."/".$dr1;
  mkdir $dest_dr_chm."/".$dr1."/html", 0;
  mkdir $dest_dr_chm."/".$dr1."/distr_chm_plugins".$dr3, 0;
  mktree($dest_dr_chm."/".$dr1."/meta");

  print "\n\n  -- translating meta into html\n\n";
  $id = 1;
  fix($dest_dr_chm."/".$dr1."/meta");
  print "total $id win32 links...\n";

  system "rm -f -r ".$dest_dr_chm."/".$dr1."/meta";

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

foreach(sort keys %hrefs){
  print OUT <<xx;
  <LI> <OBJECT type="text/sitemap">
    <param name="Name" value="$_">
    <param name="Local" value="$hrefs{$_}">
    </OBJECT>
xx
};

foreach(sort keys %hrefs2){
  if ($hrefs{$_})
  {
    print OUT <<xx;
  <LI> <OBJECT type="text/sitemap">
    <param name="Name" value="$_ (Macros)">
    <param name="Local" value="$hrefs2{$_}">
    </OBJECT>
xx
  }
  else
  {
    print OUT <<xx;
  <LI> <OBJECT type="text/sitemap">
    <param name="Name" value="$_">
    <param name="Local" value="$hrefs2{$_}">
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
# ������� ��������� ���� �� <h1> � <a name="">..</a>
sub srch
{
 local($dr1) = @_[0];
 local($dr) = $dr1."/";
 printf "\n$dr  ";
 foreach $file (`ls --ignore=articles $dr1`){
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
     $ll = "";
     print ".";
     foreach(@FData){
       if (/<h1>(.+?)<\/h1>/i){
         $x1 = $1;
         $x1 =~ s/(&quot;)|[\/\'\"<>]//g;
#         $x1 =~ /(.{1,50})/;
#         $x1 = $1;
#         if (length($x1)==50){
#           $x1 =~ /^(.*)\s([a-zA-Z�-��-�]+)$/;
#           $x1 = $1."...";
#         };
         if ($macro==0)
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
#           $x1 =~ /^(.*)\s([a-zA-Z�-��-�]+)$/;
#           $x1 = $1."...";
#         };
         if ($macro==0)
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
 foreach(`ls -1 $dr1`){
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
 foreach(`ls -1 $dr1`){
   chomp;
   if (-d $dr.$_){
     $dhtml = $dr;
     $dhtml =~ s/\/meta\//\/html\//;
     mkdir($dhtml.$_, 0) if (!-d $dhtml.$_);

     mktree("$dr$_");
   };
 };
};
