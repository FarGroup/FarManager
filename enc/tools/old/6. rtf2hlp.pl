#################################################################
# needs MS Word 2000 generated files...
# use _files.bas to generate them ...
# adds indexes, local and extern links, images(gif) and keywords,
# substitutes all win32 html links to win32.id
# руками не трогать - сам не знаю как работает!

$dir = "../rtf";
$win32 = "./api/win32.id";

cachew32();
fix($dir);

sub cachew32
{
  print "Collection win32.id...";
  open W, "$win32" or print "cant find $win32\n";
  @data = <W>;
  foreach $d (@data){
    chomp($d);
    if ($d =~ /.*?\@win32/){
      $d =~ s/\s//g;
      $win32hache{$name} = $d;
    }else{
      $name = lc($d);
    };
  };
  close W;
  print " done\n";
};

sub fix
{
 local($dr1) = @_[0];
 local($dr) = $dr1."/";
 printf "$dr\n";
# foreach $fn (`ls --ignore=CVS -1 $dr1`){
#   chomp $fn;
 foreach (`ls --ignore=CVS -1 $dr1`){
   chomp;
   local($fn) = $_;
   printf "**** Processed: $dr$fn ****\n";
   $fn =~ /(.*?)\.rtf/;
   $link = "$1.html";
   printf "{ ($dr$fn)\n";
   fix("$dr$fn") if (-d $dr.$fn);
   printf "} ($dr$fn)\n";

   printf "0. $fn   $link\n";

   if ($fn =~ /\.rtf$/){
     open F, $dr.$fn;
     @FData = <F>;
     close F;

     if ($FData[0] =~ /\\perldone/){
       print "! skip - $fn \n";
       next;
     }

     open F, ">$dr$fn";

     $FData[0] =~ s/\\ansi/\\ansi\\perldone/;
     $FData = join '', @FData;

     $title = $1 if ($FData =~ /{\\title (.*?)\}/s);

     if ($FData =~ /{\\info/){
       $FData = "$`#{\\footnote $dr$link}\${\\footnote $title}K{\\footnote $title}\n\n{\\info$'";
     };

     $ok = 1;

     while($ok){
       $ok = 0;
       if ($FData =~ /({\\\*\\bkmkstart ([^{}]*?)}([^{}]*?)})/s){
         $d = $2, $x = $3;
         $FData =~ s/{\\\*\\bkmkstart ([^{}]*?})([^{}]*?})/{ \\\*\\bkmkstart\1#{\\footnote $dr$link\!$d}K{\\footnote $x}\2/s;
         $ok = 1;
       }elsif ($FData =~ /({\\\*\\bkmkstart ([^{}]*?)})/s){
         $FData =~ s/{\\\*\\bkmkstart ([^{}]*?)}/{ \\\*\\bkmkstart  \1}\#{\\footnote $dr$link\!\1}K{\\footnote \1}/s;
         $ok = 1;
       };

       if ($FData =~ /INCLUDEPICTURE \s\s \S (.*?\.gif)/xs){
         $img = $1;
         $img =~ /\w+\.gif/;
         $img = $&;
         $img = relpos($dr.$fn, "../images/".$img);
         $FData =~ s/(\{[^{]*\{[^{]*\{ [^{]*? INCLUDEPICTURE \s\s\S (.*?\.gif)")/\\{bmc $img\\}\1/sx;
         $FData =~ s/INCLUDEPICTURE  (\S)/INCLUDEPICTURE   \1/;
         $ok = 1;
       }


       if ($FData =~ /HYPERLINK\s "([^"]*?)" (\s \\\\l \s "(.*?)")?/sx){
         $hl = $1;
         $hlpos = length($`);
         $hl =~ s/\\\w+\s?//g;
         $hl = $link if ($hl eq "");
         $hl = "$hl!$3" if ($3 ne "");
#         $hl =~ s/\.\./___/g;
#         $hl =~ s/\//!/g;
#         $hl =~ s/\.html/_html/g;
print "1. $hl\n";
         $FData =~ s/HYPERLINK "/HYPERLINK  "/;
         $ok = 1;
       };

       $wh = 1;
       while($wh){
         $wh = 0;
         if ($FData =~ /^(.*?){\\fldrslt(.*)$/xs && $hl ne ""){
           $bb = $1;
           $ll = $2;
print "2. % $hlpos - ".length($bb)."\n";
           if ($hlpos > length($bb)){
             $wh = 1;
             $FData =~ s/{\\fldrslt/{ \\fldrslt/;
             next;
           };
           $addr = tofullpath($dr, $hl);
           $addr = "!EF(`$hl',`',1,`')" if ($hl =~ /^(mailto|http|ftp|news)/i);
print "3. $hl  ++ $addr\n";
           if ($hl =~ /win32\/([\w_\d\s]+)/i){
             $fref = lc($1);
             $addr = $win32hache{$fref};
             $addr = "win32hlp-notfound-$fref" if ($addr eq "");
print "4. $fref - $addr\n";
           };
           $br = 1;
           for($j = 0; $j <= length($ll); $j++){
             $br++ if (substr($ll, $j, 1) eq '{');
             $br-- if (substr($ll, $j, 1) eq '}');
             last if (!$br);
           };
           substr($ll, $j, 1) = "\{\\v \%$addr\}\}";
           $FData = $bb."{\\strike\\fldrslt $ll";
           $hl = "";
           $ok = 1;
         };
       };
     };

     print F $FData;
     close F;
   };
 };
};

sub tofullpath
{
  local($dir) = @_[0];
  local($f) = @_[1];

  $res = $dir.$f;
  while($res =~ s/\w+\/\.\.\///){};
  return $res;
};

sub relpos
{
local ($base, $ref);
  $base = @_[0];  #  lala/base.html
  $ref = @_[1];   #  ss.html
  $res = "";

  while(1){
    $i = index($base,"/");
    $j = index($ref,"/");
print "6. relpos() :: $base-$i $ref-$j \n";
    if ($i == $j && substr($base, 0, $i) eq substr($ref, 0, $i)){
      $base = substr($base, $i+1);
      $ref = substr($ref, $i+1);
    }else{
      last;
    };
  };
  $base =~ s/([^\/]*?)\//..\//g;
  $base =~ s/[^\/]*$//g;
  $res = $base.$ref;
  return $res;
};
