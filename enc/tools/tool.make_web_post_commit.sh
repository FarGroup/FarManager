#!/bin/sh
#
# For use as a post commit hook in svn (environment not set)
# $1 = path to repository
# $2 = number of revision commited
#

/usr/bin/svnlook changed "$1" -r "$2" | /bin/grep trunk/enc
if [ $? -ne 0 ]; then exit 0; fi

/usr/bin/svn co -q --force $1/trunk/enc/ /var/tmp/enc

# need to set up path as the script will try to run svn
export PATH=$PATH:/usr/bin
cd /var/tmp/enc/tools && /usr/bin/python ./tool.make_inet.py
if [ $? -ne 0 ]; then exit 0; fi
cd /

/bin/rm -Rf /var/www/api/*
/bin/cp -Rf /var/tmp/enc/build/inet/* /var/www/api/
/bin/rm -Rf /var/tmp/enc
