#! /bin/bash
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

declare -a arr=("test00" "test01" "test01x" "test01a" "test07" "test02"  ) #"test04""test02a" "test03" "test05" "test06" "test08" "test09" "test10" "test11")

echo "#############################################"
for i in "${arr[@]}"
do
    x=`./$i > output && diff outcomes/$i output | wc -l`
    if [[ $x != 0 ]]
    then
        echo -e "${RED}$i has WRONG output${NC}"
else
	echo -e "${GREEN}$i has correct output${NC}"
    fi
    v=`valgrind ./$i |& grep "LEAK SUMMARY" | wc -l`
#    if [ $v != 0 ]
#    then
#        echo -e "${RED}$i has MEMORY LEAKS${NC}"
#else
#	echo -e "${GREEN}$i has no memory leaks${NC}"
#    fi
done
echo "#############################################"
