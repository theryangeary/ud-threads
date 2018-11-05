#!/bin/sh
while
    echo "--------------------------------------------------------------------------------"
    date
    echo "Files modified - remaking"
    make tests
    inotifywait -qqre modify .
do
    :
done
