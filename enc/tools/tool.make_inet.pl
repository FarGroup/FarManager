
print " -- preparing inet project --\n";

mkdir "../inet/", 0;
system "cp -R -f ../meta/* ../inet";

system "cp -f ./inet/index.html ../inet/index.html";
system "cp -f ./inet/index_e.html ../inet/index_e.html";
system "cp -f ./inet/notfound.html ../inet/notfound.html";

print " -- deleting win-charset tags --\n";

$lev = -1;
fix("../inet");

print " -- now convert manually all files in ../inet/ to koi8 \n";


sub fix
{
 $lev++;

 local($dr1) = @_[0];
 local($dr)=$dr1."/";
 printf "$dr\n";

 foreach(`ls --ignore=CVS -1 $dr1`){
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
