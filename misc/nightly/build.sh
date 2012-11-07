#!/bin/sh

./far.sh &> logs/far && \

./plugins.sh &> logs/plugins && \

./enc.sh &> logs/enc && \

./docs.sh &> logs/docs && \

./publish.sh &> logs/publish
