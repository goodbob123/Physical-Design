cd r12943117_pa3/
make
cd ..

for ((i = 0; i <= 10000; i++)); do
        echo "seed = $i"
        { time ./r12943117_pa3/bin/place -aux r12943117_pa3/benchmark/ibm01/ibm01-cu85.aux $i > 01.log; } 2>>01.log
        HPWL=$(grep "=======" -A 1 01.log | tail -n 1 | awk '{print $2}')
        real_time=$(grep "^real" 01.log | awk '{print $2}' | sed 's/^0m//; s/s$//')
        if ! grep -q "legalization success!" 01.log; then
            perl ./r12943117_pa3/check_density_target_01.pl ./r12943117_pa3/benchmark/ibm01/ibm01.nodes ./ibm01-cu85.gp.pl ./r12943117_pa3/benchmark/ibm01/ibm01-cu85.scl > over01.log
            scaled_overflow=$(grep "Scaled Overflow per bin" over01.log | awk '{print $5}')
            HPWL=$(echo "$HPWL * (1 + $scaled_overflow)" | bc)
        fi
        ./evaluator/evaluator.sh ./r12943117_pa3/benchmark/ibm01/ibm01-cu85.aux $HPWL $real_time > 01_result
        final_score_1=$(grep "^[[:space:]]*=[[:space:]]*" 01_result | tail -n 1 | awk -F'=' '{print $2}' | xargs)

        { time ./r12943117_pa3/bin/place -aux r12943117_pa3/benchmark/ibm05/ibm05.aux $i > 05.log; } 2>>05.log
        HPWL=$(grep "=======" -A 1 05.log | tail -n 1 | awk '{print $2}')
        real_time=$(grep "^real" 05.log | awk '{print $2}' | sed 's/^0m//; s/s$//')
        if ! grep -q "legalization success!" 05.log; then
            perl ./r12943117_pa3/check_density_target_05.pl ./r12943117_pa3/benchmark/ibm05/ibm05.nodes ./ibm05.gp.pl ./r12943117_pa3/benchmark/ibm05/ibm05.scl > over05.log
            scaled_overflow=$(grep "Scaled Overflow per bin" over05.log | awk '{print $5}')
            HPWL=$(echo "$HPWL * (1 + $scaled_overflow)" | bc)
        fi
        ./evaluator/evaluator.sh ./r12943117_pa3/benchmark/ibm05/ibm05.aux $HPWL $real_time > 05_result
        final_score_2=$(grep "^[[:space:]]*=[[:space:]]*" 05_result | tail -n 1 | awk -F'=' '{print $2}' | xargs)
        final_score=$(echo "$final_score_1 + $final_score_2" | bc)
        best_score=$(head -n 1 ./best/best_final | awk '{print $1}')
        if awk "BEGIN {exit !($final_score > $best_score)}"; then
            echo "better score $final_score found!"
            echo "$final_score" > best/best_final
            echo "N = 160, alpha = 0.2, seed = $i" >> best/best_final
            if ! grep -q "legalization success!" 05.log; then
                mv ibm01-cu85.gp.pl ./best/ibm01-cu85.gp.pl
                mv ibm05.gp.pl ./best/ibm05.gp.pl
            else
                mv ibm01-cu85.dp.pl ./best/ibm01-cu85.dp.pl
                mv ibm05.dp.pl ./best/ibm05.dp.pl
            fi
        fi
done