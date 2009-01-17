BEGIN {
  test = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
}

{
  if (index($0,"Copyright") && match($0, /^(.*")(.*)(".*)$/, a))
  {
    printf "%s", a[1]
    enc(a[2], 17)
    print a[3]
  }
  else
  {
    print $0
  }
}

function enc(str, Xor)
{
  for (i = 1; i <= length(str); i = i + 1)
  {
    ch = index(test,substr(str,i,1)) + 31
    #new lines are marked by $
    if (ch == 36)
      ch = 10
    save = ch
    ch = or(xor(ch,Xor),128)
    printf("\\x%x",ch)
    Xor = xor(Xor,save)
  }
}
