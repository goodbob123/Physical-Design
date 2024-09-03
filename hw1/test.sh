time ./r12943117_pa1/bin/fm input_pa1/input_0.dat output_0.dat
time ./r12943117_pa1/bin/fm input_pa1/input_1.dat output_1.dat
time ./r12943117_pa1/bin/fm input_pa1/input_2.dat output_2.dat
time ./r12943117_pa1/bin/fm input_pa1/input_3.dat output_3.dat
time ./r12943117_pa1/bin/fm input_pa1/input_4.dat output_4.dat
time ./r12943117_pa1/bin/fm input_pa1/input_5.dat output_5.dat

./evaluator/evaluator.sh ./input_pa1/input_0.dat ./output_0.dat
./evaluator/evaluator.sh ./input_pa1/input_1.dat ./output_1.dat
./evaluator/evaluator.sh ./input_pa1/input_2.dat ./output_2.dat
./evaluator/evaluator.sh ./input_pa1/input_3.dat ./output_3.dat
./evaluator/evaluator.sh ./input_pa1/input_4.dat ./output_4.dat
./evaluator/evaluator.sh ./input_pa1/input_5.dat ./output_5.dat