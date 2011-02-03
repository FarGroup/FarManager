BEGIN {}
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
    ch = asc(substr(str,i,1))
    #new lines are marked by $
    if (ch == 36)
      ch = 10
    save = ch
    ch = xor(ch,Xor)
    printf("\\x%x",ch)
    Xor = xor(Xor,save)
  }
}

function asc(c, tchar, ascval, b)
{
  if (c == "")
    return ""
  c = substr(c, 1, 1)
  ascval = b = 128
  while ((tchar = sprintf("%c", ascval)) != c)
    ascval += (b /= 2) * (tchar < c) ? 1 : -1
  return int(ascval)
}
