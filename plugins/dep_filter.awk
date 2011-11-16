BEGIN {
   patt = "^"n"\\.o[ \t]*:";
   repl = r"/"n".o " r"/"n".d:";
}

{
   gsub(patt,repl);
   print;
}
