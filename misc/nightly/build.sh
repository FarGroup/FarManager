#!/bin/bash

function run {
  if ! ./$1.sh &> logs/$1; then
    echo "$1.sh failed"
    return 1
  fi
}

WINEDEBUG=-all
export WINEDEBUG

if cd far.git && git pull && cd ..; then

	#start mspdbsrv.exe manually before compilation, with an infinite timeout
	#prevents compilation being stuck when using cmake
	#wine c:/VC_current/bin/Hostx86/x86/mspdbsrv.exe -start -spawn -shutdowntime -1 &> /dev/null &


	run "far" && run "plugins" && run "colorer" && run "netbox" && run "enc" && run "docs" && run "publish"

	#kill mspdbsrv.exe as it is no longer needed
	#kill `pidof mspdbsrv.exe 'c:\VC_current\bin\Hostx86\x86\mspdbsrv.exe'`
else
	echo "git pull failed"
	exit 1
fi
