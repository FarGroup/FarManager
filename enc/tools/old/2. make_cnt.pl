#####################################################
# from hhc to cnt


# used to convert from chm content tree into hlp content
$cnt = "../pluginsr.cnt";
$cntcp = "../distr_hlp_pluginsr/pluginsr.cnt";
$chmcnt = "../pluginsr.hhc";

makecnt();

sub makecnt()
{
  open F, $chmcnt;
  @data = <F>;
  close F;

  open F, ">$cnt";
  print F <<xx;
:Base PLUGINSR.HLP
:Title FAR plugins programming - Encyclopedia of Developer
xx
  $lev = 0;
  foreach $el(@data){
    $lev++ if ($el =~ /<ul>/i);
    $lev-- if ($el =~ /<\/ul>/i);
    if ($el =~ /\s+<\/OBJECT>/){
      print ".";
      $local =~ s/html\///;
      next if ($local =~ /win32\//);
      if ($local =~ /\/index\.html/){
        print F "$lev $name\n";
        print F ($lev+1)." $name=$local\n";
      }else{;
        $i = $local eq ""?"":"=";
        print F "$lev $name".$i."$local\n";
      };
      $name = "";
      $local = "";
    };
    $name = $1 if ($el =~ /<param name="Name" value="(.*?)">/s);
    $local = $1 if ($el =~ /<param name="Local" value="(.*?)">/s);
  };
  print "\ndone\n";
  close F;
  system "cp -f $cnt $cntcp";
};
