#!/bin/sh

./far.sh &> logs/far && \

./plugins.sh &> logs/plugins && \

./colorer.sh &> logs/colorer && \

./enc.sh &> logs/enc && \

./docs.sh &> logs/docs && \

./publish.sh &> logs/publish
