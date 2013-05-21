#!/bin/bash

#start mspdbsrv.exe manually before compilation, with an infinite timeout
#prevents compilation being stuck when using cmake
wine c:/VC10/bin/mspdbsrv.exe -start -spawn -shutdowntime -1 &> /dev/null &

./far.sh &> logs/far && \

./plugins.sh &> logs/plugins && \

./colorer.sh &> logs/colorer && \

./netbox.sh &> logs/netbox && \

./enc.sh &> logs/enc && \

./docs.sh &> logs/docs && \

./publish.sh &> logs/publish
