#!/bin/sh

./far.sh &> logfar

./plugins.sh &> logplugins

#./enc.sh &> logenc

./docs.sh &> logdocs

./publish.sh &> logpublish
