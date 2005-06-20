
############################################################
# Генерит индексный файл по <h1> и <a name="">..</a>
# упорядочивать не обязательно кстати :)
# Cail Lomecb <ruiv@uic.nnov.ru>

$fname="../pluginsr.hhk";
$rel = "html";
$dir = "../$rel";

srch($dir);

sub srch
{
 local($dr1) = @_[0];
 local($dr) = $dr1."/";
 printf "\n$dr  ";
 foreach $file (`ls --ignore=CVS --ignore=articles $dr1`){
   chomp $file;
   if (-d $dr.$file){
     srch("$dr$file");
   }
   if ($file =~ /\.html$/ && $file !~ /faq\.html/){
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
         $x1 =~ /(.{1,50})/;
         $x1 = $1;
         if (length($x1)==50){
           $x1 =~ /^(.*)\s([a-zA-Zю-ъЮ-Ъ]+)$/;
           $x1 = $1."...";
         };
         $hrefs{$x1} = $fn;
       };
       if (/<a.+?name=[\"\'](.+?)[\"\']>(.+?)<\/a>/i){
         $x2 = $1;
         $x1 = $2;
         $x1 =~ s/(&quot;)|[\/\'\"<>]//g;
         $x1 =~ /(.{1,50})/;
         $x1 = $1;
         if (length($x1)==50){
           $x1 =~ /^(.*)\s([a-zA-Zю-ъЮ-Ъ]+)$/;
           $x1 = $1."...";
         };
         $hrefs{$x1} = $fn."#$x2";
       };
     };
   };
 };
};

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
    <param name="Name" value="$_">
    <param name="Local" value="$hrefs{$_}">
    </OBJECT>
xx
};
#<param name="DefaultTopic" VALUE="../notopic.htm">

print OUT <<xx;
</UL>
</BODY></HTML>
xx
close OUT;
