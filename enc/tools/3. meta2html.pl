#
#translates meta into html - makes msdn links
#

$meta = "../meta";
$html = "../html/";
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
#<object id=help$id type="application\/x-oleobject" classid="clsid:adb880a6-d8ff-11cf-9377-00aa003b7a11">
#<param name="Command" value="WinHelp">
#<param name="Item1" value="win32.hlp">
#<param name="Item2" value="$win32hache{lc($1)}">
#<\/object>

     print F $_;
   };
   close F;
 };
};
