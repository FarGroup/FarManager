
do "clear.pl";

print "\n\n  -- making vbasic files and hlp includes --\n\n";
do "1. make_bas_inc.pl";

print "\n  -- making hlp content file --\n\n";
do "2. make_cnt.pl";

print "  -- making directories tree.\n\n";

mkdir("../rtf", 0);
mkdir("../html", 0);
mkrtf("../meta");

sub mkrtf
{
 local($dr1) = @_[0];
 local($dr) = $dr1."/";
 printf "$dr\n";
 foreach(`ls --ignore=CVS -1 $dr1`){
   chomp;
   if (-d $dr.$_){
     $dhtml = $dr;
     $dhtml =~ s/^\.\.\/meta/\.\.\/html/;
     mkdir($dhtml.$_, 0) if (!-d $dhtml.$_);

     $drtf = $dr;
     $drtf =~ s/^\.\.\/meta/\.\.\/rtf/;
     mkdir($drtf.$_, 0) if (!-d $drtf.$_);

     mkrtf("$dr$_");
   };
 };
};

print "\n\n  -- translating meta into html\n\n";

do "3. meta2html.pl";

print "\n  -- making chm indexes --\n\n";

do "4. generate_indexes.pl";

print "\n\n  -- translating meta into html for rtf\n\n";

do "5. meta2html_rtf.pl";
do "CScript.exe //nologo html2rtf.vbs";

print "\n\n  -- preparation stage finished\n     you can build chm project\n\n";

print "  -- run that fucking word and make rtf's - than run \"6. rtf2hlp.pl\"";
