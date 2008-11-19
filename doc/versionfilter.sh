#!/bin/sh

cd `dirname $0`/..
file="${1/$PWD\//}"
git log -n1 --pretty=oneline -- "${file}" | head -n1 | cut -d ' ' -f 1
