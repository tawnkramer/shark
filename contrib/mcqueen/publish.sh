#!/bin/bash
if [ -z "$1" ]
	then
		echo "No check-in message"
		exit
fi
git add -Afv *
git commit -a -m '$1'
git push origin master

