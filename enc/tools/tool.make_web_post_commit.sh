#!/bin/sh
#
# For use as a post commit hook from webservice
# $1 = path to repository
# $2 = number of revision commited
#

svn log -r "$2" -v "$1" | /bin/grep trunk/enc
if [ $? -ne 0 ]; then exit 0; fi

svn co -q --force $1/trunk/enc/ /var/tmp/enc

cd /var/tmp/enc/tools && python ./tool.make_inet.py
if [ $? -ne 0 ]; then exit 0; fi
cd /

rm -Rf /var/www/api/*
cp -Rf /var/tmp/enc/build/inet/* /var/www/api/
rm -Rf /var/tmp/enc
