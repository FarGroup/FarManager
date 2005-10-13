
#######################################
# проверяет ссылки по всем html-ям

$dir = "../enc_rus/meta";
srch($dir);

$dir = "../enc_eng/meta";
srch($dir);


sub srch
{
 local($dr1) = @_[0];
 local($dr)=$dr1."/";
 #printf "$dr1\n";
 foreach(`ls --ignore=.svn $dr1`){
   chomp;
   if (-d $dr.$_){
     srch("$dr$_");
   }
   if (/\.html$/){
     $fn = $_;
     open F, $dr.$_;
     @FData = <F>;
     close F;
     foreach(@FData){
       if (/(href|src)=["'] ([^\'\"\#]+?) [\'\"\#]/xi){
         $ff = $2;
         unless($ff=~/http|mailto|ftp|news/ || -r $dr.$ff){
           if(!($ff =~ /win32\//)){
             print "! $dr$fn - $ff\n";
           };
         };
       };
     };
   };
 };
};
