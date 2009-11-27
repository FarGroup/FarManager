BEGIN {
  seen = 0;
}

{
  if (seen == 0)
  {
    if (match($0, /^t\-rex 12\.12\.2008 18\:21\:17 \+0200 \- build 2478/, a) != 0)
    {
      seen = 1;
    }
    else
    {
      print $0;
    }
  }
}
