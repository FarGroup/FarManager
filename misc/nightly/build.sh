#!/bin/bash

./far.sh &> logs/far && \

./plugins.sh &> logs/plugins && \

./colorer.sh &> logs/colorer && \

./netbox.sh &> logs/netbox && \

./enc.sh &> logs/enc && \

./docs.sh &> logs/docs && \

./publish.sh &> logs/publish
