cd r12943117_pa2/; make; cd ..;
./r12943117_pa2/bin/fp 0.5 input_pa2/ami33.block input_pa2/ami33.nets ami33.rpt
./r12943117_pa2/bin/fp 0.5 input_pa2/ami49.block input_pa2/ami49.nets ami49.rpt
./r12943117_pa2/bin/fp 0.5 input_pa2/apte.block input_pa2/apte.nets apte.rpt
./r12943117_pa2/bin/fp 0.5 input_pa2/hp.block input_pa2/hp.nets hp.rpt
./r12943117_pa2/bin/fp 0.5 input_pa2/xerox.block input_pa2/xerox.nets xerox.rpt

./evaluator/evaluator.sh input_pa2/ami33.block input_pa2/ami33.nets ami33.rpt 0.5
./evaluator/evaluator.sh input_pa2/ami49.block input_pa2/ami49.nets ami49.rpt 0.5
./evaluator/evaluator.sh input_pa2/apte.block input_pa2/apte.nets apte.rpt 0.5
./evaluator/evaluator.sh input_pa2/hp.block input_pa2/hp.nets hp.rpt 0.5
./evaluator/evaluator.sh input_pa2/xerox.block input_pa2/xerox.nets xerox.rpt 0.5