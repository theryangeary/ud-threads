#! /bin/bash

declare -a arr=("test00" "test01" "test01x" "test01a" "test02" "test02a") #"test03" "test04" "test05" "test06" "test07" "test08" "test09" "test10" "test11")

echo "#############################################"
for i in "${arr[@]}"
do
    x=`./$i > output && diff outcomes/$i output | wc -l`
    if [ $x != 0 ]
    then
        echo "$i has wrong output"
    fi
    v=`valgrind ./$i |& grep "LEAK SUMMARY" | wc -l`
    if [ $v != 0 ]
    then
        echo "$i has memory leaks"
    fi
done
echo "#############################################"
