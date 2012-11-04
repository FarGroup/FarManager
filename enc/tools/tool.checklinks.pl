#!/usr/bin/perl -w

#######################################
# проверяет ссылки по всем html-ям
use strict;

my $toolpath = "";
if($^O eq "MSWin32"){
  $toolpath="./tools/";
}

#my @dirs=("../enc_rus/meta","../enc_eng/meta","../enc_rus2/meta");
my @dirs=("../enc_rus3.work/meta");
foreach my $dir (@dirs) {
  srch($dir);
}

#$dir = "../enc_eng2/meta";
#srch($dir);

sub srch
{
 my $dr1 = shift;
 my $dr=$dr1."/";
 #printf "$dr1\n";
 my $ls=$toolpath."ls --ignore=.svn ".$dr1;
 foreach my $fn (`$ls`){
   chomp($fn);
   my $ffn=$dr.$fn;
   if (-d $ffn) {
     srch($ffn);
   } elsif ($fn=~/\.html$/) {
     open my $f, '<', $ffn;
     my @FData = <$f>;
     close $f;
     foreach (@FData) {
       if (/(href|src)=["'] ([^\'\"\#]+?) [\'\"\#]/xi){
         my $ff = $2;
         unless($ff=~/http|mailto|ftp|news/ || -r $dr.$ff){
           if(!($ff =~ /win32\//)){
             print "! $ffn - $ff\n";
           }
         }
       }
     }
   }
 }
}
