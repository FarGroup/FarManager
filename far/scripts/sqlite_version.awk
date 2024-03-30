BEGIN {
  getline baseline < target
}

{
  str = "#define SQLITE_VERSION_NUMBER "
  {
    if (substr($0, 1, length(str)) == str)
    {
      number = substr($0, length(str) + 1)
      x = int(number / 1000000)
      y = int(number % 1000000 / 1000)
      z = int(number % 1000)
      final_str = "#define SQLITE_RESOURCE_VERSION " x "," y "," z
      if (final_str != baseline)
          print final_str > target
      exit
    }
  }
}
