#!/bin/bash
#
# For use as a post commit hook from webservice
# $1 = path to repository
# $2 = number of revision commited
#

( 
	flock 200

	svn log -r "$2" -v "$1" | grep trunk/enc
	if [ $? -ne 0 ]; then exit 0; fi

	svn co -q --force $1/trunk/enc/ /var/tmp/enc

	cd /var/tmp/enc/tools && python ./tool.make_inet.py
	if [ $? -ne 0 ]; then exit 0; fi
	cd /

	rm -Rf /var/www/api/*
	cp -Rf /var/tmp/enc/build/inet/* /var/www/api/
	rm -Rf /var/tmp/enc 

) 200>/var/tmp/tool.make_web_post_commit.lock
