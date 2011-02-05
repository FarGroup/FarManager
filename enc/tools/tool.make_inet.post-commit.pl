#!/usr/bin/perl -w

# Turn on warnings the best way depending on the Perl version.
BEGIN {
  if ( $] >= 5.006_000)
    { require warnings; import warnings; }
  else
    { $^W = 1; }
}

use strict;
use Carp;

######################################################################
# Configuration section.

# Svnlook path.
my $svnlook = "/usr/bin/svnlook";
my $svn = "/usr/bin/svn";
my $sendmail = "/usr/sbin/sendmail";

# End of Configuration section.
######################################################################

# Since the path to svnlook depends upon the local installation
# preferences, check that the required programs exist to insure that
# the administrator has set up the script properly.
{
  my $ok = 1;
  foreach my $program ($svnlook,$svn,$sendmail)
    {
      if (-e $program)
        {
          unless (-x $program)
            {
              warn "$0: required program `$program' is not executable, ",
                   "edit $0.\n";
              $ok = 0;
            }
        }
      else
        {
          warn "$0: required program `$program' does not exist, edit $0.\n";
          $ok = 0;
        }
    }
  exit 1 unless $ok;
}


######################################################################
# Initial setup/command-line handling.

my @project_settings_list;
my $current_project;

# Process the command line arguments till there are none left.  The
# first two arguments that are not used by a command line option are
# the repository path and the revision number.
my $repos;
my $rev;

# This hash matches the command line option to the hash key in the
# project.  If a key exists but has a false value (''), then the
# command line option is allowed but requires special handling.
my %opt_to_hash_key = ('-m'     => '');

while (@ARGV)
  {
    my $arg = shift @ARGV;
    if ($arg =~ /^-/)
      {
        my $hash_key = $opt_to_hash_key{$arg};
        unless (defined $hash_key)
          {
            die "$0: command line option `$arg' is not recognized.\n";
          }

        unless (@ARGV)
          {
            die "$0: command line option `$arg' is missing a value.\n";
          }
        my $value = shift @ARGV;

        if ($hash_key)
          {
            $current_project->{$hash_key} = $value;
          }
        else
          {
            # Here handle -m.
            unless ($arg eq '-m')
              {
                die "$0: internal error: should only handle -m here.\n";
              }
            $current_project                = &new_project;
            $current_project->{match_regex} = $value;
            push(@project_settings_list, $current_project);
          }
      }
    elsif ($arg =~ /^-/)
      {
        die "$0: command line option `$arg' is not recognized.\n";
      }
    else
      {
        if (! defined $repos)
          {
            $repos = $arg;
          }
        elsif (! defined $rev)
          {
            $rev = $arg;
          }
      }
  }

# If the revision number is undefined, then there were not enough
# command line arguments.
&usage("$0: too few arguments.") unless defined $rev;

# Check the validity of the command line arguments.  Check that the
# revision is an integer greater than 0 and that the repository
# directory exists.
unless ($rev =~ /^\d+/ and $rev > 0)
  {
    &usage("$0: revision number `$rev' must be an integer > 0.");
  }
unless (-e $repos)
  {
    &usage("$0: repos directory `$repos' does not exist.");
  }
unless (-d _)
  {
    &usage("$0: repos directory `$repos' is not a directory.");
  }

# Check that all of the regular expressions can be compiled and
# compile them.
{
  my $ok = 1;
  for (my $i=0; $i<@project_settings_list; ++$i)
    {
      my $match_regex = $project_settings_list[$i]->{match_regex};

      # To help users that automatically write regular expressions
      # that match the root directory using ^/, remove the / character
      # because subversion paths, while they start at the root level,
      # do not begin with a /.
      $match_regex =~ s#^\^/#^#;

      my $match_re;
      eval { $match_re = qr/$match_regex/ };
      if ($@)
        {
          warn "$0: -m regex #$i `$match_regex' does not compile:\n$@\n";
          $ok = 0;
          next;
        }
      $project_settings_list[$i]->{match_re} = $match_re;
    }
  exit 1 unless $ok;
}

######################################################################
# Harvest data using svnlook.

# Change into suitable directory so that svnlook diff can create its .svnlook
# directory. This could be removed - it's only for compatibility with
# 1.0.x svnlook - from 1.1.0, svnlook will be sensible about choosing a
# temporary directory all by itself.
my $tmp_dir = ((defined($ENV{'TEMP'}) && -d $ENV{'TEMP'}) ?
               $ENV{'TEMP'} : '/tmp');
chdir($tmp_dir)
  or die "$0: cannot chdir `$tmp_dir': $!\n";

# Figure out what directories have changed using svnlook.
my @dirschanged = &read_from_process($svnlook, 'dirs-changed', $repos,
                                     '-r', $rev);

# Lose the trailing slash in the directory names if one exists, except
# in the case of '/'.
my $rootchanged = 0;
for (my $i=0; $i<@dirschanged; ++$i)
  {
    if ($dirschanged[$i] eq '/')
      {
        $rootchanged = 1;
      }
    else
      {
        $dirschanged[$i] =~ s#^(.+)[/\\]$#$1#;
      }
  }

# Figure out what files have changed using svnlook.
my @svnlooklines = &read_from_process($svnlook, 'changed', $repos, '-r', $rev);

# Parse the changed nodes.
my @adds;
my @dels;
my @mods;
foreach my $line (@svnlooklines)
  {
    my $path = '';
    my $code = '';

    # Split the line up into the modification code and path, ignoring
    # property modifications.
    if ($line =~ /^(.).  (.*)$/)
      {
        $code = $1;
        $path = $2;
      }

    if ($code eq 'A')
      {
        push(@adds, $path);
      }
    elsif ($code eq 'D')
      {
        push(@dels, $path);
      }
    else
      {
        push(@mods, $path);
      }
  }

######################################################################
# Modified directory name collapsing.

# Collapse the list of changed directories only if the root directory
# was not modified, because otherwise everything is under root and
# there's no point in collapsing the directories, and only if more
# than one directory was modified.
my $commondir = '';
my @dirschanged_orig = @dirschanged;
if (!$rootchanged and @dirschanged > 1)
  {
    my $firstline    = shift @dirschanged;
    my @commonpieces = split('/', $firstline);
    foreach my $line (@dirschanged)
      {
        my @pieces = split('/', $line);
        my $i = 0;
        while ($i < @pieces and $i < @commonpieces)
          {
            if ($pieces[$i] ne $commonpieces[$i])
              {
                splice(@commonpieces, $i, @commonpieces - $i);
                last;
              }
            $i++;
          }
      }
    unshift(@dirschanged, $firstline);

    if (@commonpieces)
      {
        $commondir = join('/', @commonpieces);
        my @new_dirschanged;
        foreach my $dir (@dirschanged)
          {
            if ($dir eq $commondir)
              {
                $dir = '.';
              }
            else
              {
                $dir =~ s#^\Q$commondir/\E##;
              }
            push(@new_dirschanged, $dir);
          }
        @dirschanged = @new_dirschanged;
      }
  }
my $dirlist = join(' ', @dirschanged);

######################################################################
# Assembly of log message.
# Go through each project and see if there are any matches for this
# project.  If so, send the log out.
{
  my $match = 0;
  foreach my $project (@project_settings_list)
  {
    my $match_re = $project->{match_re};
    foreach my $path (@dirschanged_orig, @adds, @dels, @mods)
    {
      if ($path =~ $match_re)
      {
        $match = 1;
        last;
      }
    }
  }
  exit 0 unless $match;
}

my $lev;

my $dest_dr            = "/var/www/api";

my $dest_dr_inet       = $dest_dr."/temp";

print "PREPARING INET PROJECT\n";

print "\n  -- clear INET\n";

system "rm -Rf ".$dest_dr."/*";

print "\n  -- making directories tree\n";

mkdir $dest_dr_inet, 0775;
mkdir $dest_dr."/images", 0775;
mkdir $dest_dr."/styles", 0775;

#mk_inet_lng("ru","rus");
#mk_inet_lng("en","eng");
mk_inet_lng("ru2","rus2");
#mk_inet_lng("en2","eng2");

system $svn." export -q --force http://localhost/svn/trunk/enc/tools/inet/ ".$dest_dr_inet."/inet";
system "cp -f ".$dest_dr_inet."/inet/index.html ".$dest_dr;
system "cp -f ".$dest_dr_inet."/inet/farenclogo.gif ".$dest_dr."/images/farenclogo.gif";
system "cp -f ".$dest_dr_inet."/inet/styles.css ".$dest_dr."/styles/styles.css";

system "rm -Rf ".$dest_dr_inet;

system "chmod -R g+w ".$dest_dr."/*";

#notify by mail of site update
my $userlist = "trexinc\@gmail.com vskirdin\@gmail.com";
my @head;
push(@head, "To: trexinc\@gmail.com, vskirdin\@gmail.com\n");
push(@head, "From: svn\@farmanager.com\n");
push(@head, "Subject: FAR-SVN: api.farmanager.com was updated!\n");
push(@head, "Content-Type: text/plain; charset=windows-1251\n");
push(@head, "Content-Transfer-Encoding: 8bit\n");
push(@head, "\n");

my @body;
push(@body, "api.farmanager.com was updated!\n");

# Open a pipe to sendmail.
my $command = "$sendmail $userlist";
if (open(SENDMAIL, "| $command"))
{
  print SENDMAIL @head, @body;
  close SENDMAIL
  or warn "$0: error in closing `$command' for writing: $!\n";
}
else
{
  warn "$0: cannot open `| $command' for writing: $!\n";
}

exit 0;

sub mk_inet_lng
{
  my $dr1 = $_[0];
  my $dr2 = $_[1];

  print " -- preparing ".$dr1." --\n";
  system $svn." export -q --force http://localhost/svn/trunk/enc/enc_".$dr2." ".$dest_dr_inet."/meta.".$dr1;
  mkdir $dest_dr."/".$dr1."/", 0775;
  system "cp -Rf ".$dest_dr_inet."/meta.".$dr1."/meta/* ".$dest_dr."/".$dr1;
  system "cp -Rf ".$dest_dr_inet."/meta.".$dr1."/images/* ".$dest_dr."/images";
  system "rm -Rf ".$dest_dr_inet."/meta.".$dr1;
  $lev = -1;
  fix($dest_dr."/".$dr1);
};

sub fix
{
 $lev++;

 my $dr1 = $_[0];
 my $dr = $dr1."/";
 printf "$dr\n";

 foreach(`ls -1 $dr1`){
   chomp;
   if (-d $dr.$_) {
     fix("$dr$_");
   }

   if (/\.html$/){
     open F, $dr.$_;
     my @FData = <F>;
     close F;

     open F, ">$dr$_";
     foreach(@FData){
       #if (/<meta\s.*?Windows-1251.*$/xi){
       #  next;
       #};
       my $back = "../"x$lev;
       s[ <a \s* href=\"win32\/ .*?>(.*?)</a> ][ <a href=\"${back}notfound.html?$1\">$1</a> ]igx;
       print F $_;
     };
     close F;
   };
 };
 $lev--;
};

# Return a new hash data structure for a new empty project that
# matches any modifications to the repository.
sub new_project
{
  return {match_regex     => '.'};
}

# Start a child process safely without using /bin/sh.
sub safe_read_from_pipe
{
  unless (@_)
    {
      croak "$0: safe_read_from_pipe passed no arguments.\n";
    }

  my $pid = open(SAFE_READ, '-|');
  unless (defined $pid)
    {
      die "$0: cannot fork: $!\n";
    }
  unless ($pid)
    {
      open(STDERR, ">&STDOUT")
        or die "$0: cannot dup STDOUT: $!\n";
      exec(@_)
        or die "$0: cannot exec `@_': $!\n";
    }
  my @output;
  while (<SAFE_READ>)
    {
      s/[\r\n]+$//;
      push(@output, $_);
    }
  close(SAFE_READ);
  my $result = $?;
  my $exit   = $result >> 8;
  my $signal = $result & 127;
  my $cd     = $result & 128 ? "with core dump" : "";
  if ($signal or $cd)
    {
      warn "$0: pipe from `@_' failed $cd: exit=$exit signal=$signal\n";
    }
  if (wantarray)
    {
      return ($result, @output);
    }
  else
    {
      return $result;
    }
}

# Use safe_read_from_pipe to start a child process safely and return
# the output if it succeeded or an error message followed by the output
# if it failed.
sub read_from_process
{
  unless (@_)
    {
      croak "$0: read_from_process passed no arguments.\n";
    }
  my ($status, @output) = &safe_read_from_pipe(@_);
  if ($status)
    {
      return ("$0: `@_' failed with this output:", @output);
    }
  else
    {
      return @output;
    }
}
