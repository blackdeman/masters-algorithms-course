
cachesize=6291456
cacheways=12
cacheline=64
#cachgrind_additional_params="--LL=$cachesize,$cacheways,$cacheline"
cachgrind_additional_params=""

tests=(1023 1024 1025 1040 1041 1050 1100 1279 1280 1281)

echo "Start : `date`"
for item in ${tests[*]}
do
    echo "Processing $item"
    ./cache_analyzed $item $cachesize $cacheways $cacheline 0 >> cache_analyzer_lru_6mb.log 2>&1 &
    ./cache_analyzed $item $cachesize $cacheways $cacheline 1 >> cache_analyzer_random_6mb.log 2>&1 &
    valgrind --tool=cachegrind $cachgrind_additional_params ./cache $item >> valgrind_6mb.log 2>&1 &
    wait
    echo "Finished $item : `date`"
done

