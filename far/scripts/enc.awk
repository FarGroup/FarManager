BEGIN {
  FS = ""
  Xor = 17
  printf "char *Copyright=\""
  test = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
}

{
  for (i = 1; i <= NF; i = i + 1)
  {
    ch = index(test,$i) + 31
    #new lines are marked by $
    if (ch == 36)
      ch = 10
    save = ch
    ch = or(xor(ch,Xor),128)
    printf ("\\x%x",ch)
    Xor = xor(Xor,save)
  }
}

END {
  print "\";"
}
