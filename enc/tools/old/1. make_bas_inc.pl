############################################################
# makes _files.bas
# ignores all win32 topics!!!

$name = "basfiles.lst";
$incname = "../pluginsr.inc";
$dir = "../meta";
$no = 0;

open F, ">$name";
open F1, ">$incname";

srch($dir);
$no--;
close F1;
close F;

sub srch
{
 local($dr1) = @_[0];
 local($dr)=$dr1."/";
 printf "$dr\n";
 foreach(`ls --ignore=CVS -1 $dr1`){
   chomp;
   if (-d $dr.$_){
     srch("$dr$_");
   }
   if (/^(.*)\.html$/){
     $fn = $1;
     $dr =~ /$dir(.*)/;

     $fn = $1.$fn;
     $fn =~ s/\//\\/;
     next if ($fn =~ /win32\\/);
     print F "$fn.html\n";
     print F "$fn.rtf\n";
     print F1 "rtf\\$fn.rtf\n";
     $no++;
   };
 };
};
