print " -- preparing inet project --\n";

mkdir "../inet/", 0;

print " -- preparing RU --\n";
system "svn export ../enc_rus ../inet/meta.ru";
mkdir "../inet/ru/", 0;
system "cp -R -f ../inet/meta.ru/meta/* ../inet/ru";

system "rm -f -r ../inet/meta.ru";

$lev = -1;
fix("../inet/ru");


print " -- preparing EN --\n";
system "svn export ../enc_eng ../inet/meta.en";
mkdir "../inet/en/", 0;
system "cp -R -f ../inet/meta.en/meta/* ../inet/en";

system "rm -f -r ../inet/meta.en";

$lev = -1;
fix("../inet/en");

#print " -- now convert manually all files in ../inet/ to koi8 \n";


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
