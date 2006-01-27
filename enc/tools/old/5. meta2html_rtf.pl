#
#translates meta into html - makes msdn links
#

$meta = "../meta";
$html = "../rtf/";
$win32 = "./api/win32.id";
$id = 1;

#cachew32();
fix($meta);

print "total $id win32 links...\n";

#sub cachew32
#{
#  open W, "$win32" or print "cant find $win32\n";
#  @data = <W>;
#  foreach $d (@data){
#    chomp($d);
#    if ($d =~ /(.*?)\@win32/){
#      $d = $1;
#      $d =~ s/\s//g;
#      $win32hache{$name} = $d;
#    }else{
#      $name = lc($d);
#    };
#  };
#  close W;
#};

sub fix
{
 local($dr1) = @_[0];
 local($dr)=$dr1."/";
 printf "$dr\n";
 foreach(`ls --ignore=CVS -1 $dr1`){
   chomp;
   if (-d $dr.$_) {
     fix("$dr$_");
   }

   $fn = $_;
   open F, $dr.$_;
   @FData = <F>;
   close F;

   $newdr = $dr;
   $newdr =~ s/\/meta\//\/rtf\//;
   open F, ">$newdr$_";
   foreach(@FData){
     while(s/styles\.css/styles\-hlp\.css/i){
       $id++;
     }
     print F $_;
   };
   close F;
 };
};
