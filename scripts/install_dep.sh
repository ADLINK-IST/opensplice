#!/bin/bash

set -e -x
cd $(dirname $0)/..

wget https://nodejs.org/dist/v8.11.1/node-v8.11.1-win-x64.zip
unzip node-v8.11.1-win-x64.zip

wget https://www.sqlite.org/2019/sqlite-amalgamation-3290000.zip
unzip sqlite-amalgamation-3290000.zip
