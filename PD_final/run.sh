#測試測資4 : ./run.sh 4
#測試測資5 : ./run.sh 5
#default測資4和5 : ./run.sh
#畫圖 : ./run.sh -draw [4|5|all]
make

if [ -n "$1" ]; then
    if [ "$1" == "-draw" ]; then
        if [ -n "$2" ]; then
            if [ $2 -eq "4" ]; then
                ./bin/CGR -draw 20 testcases/case4/case4_def testcases/case4/case4_cfg.json testcases/case4/case4.json
                python3 ./bin/draw_grid.py --input grid.csv --output grid4.png
            elif [ $2 -eq "5" ]; then
                ./bin/CGR -draw 20 testcases/case5/case5_def testcases/case5/case5_cfg.json testcases/case5/case5.json
                python3 ./bin/draw_grid.py --input grid.csv --output grid5.png
            else
                ./bin/CGR -draw 20 testcases/case4/case4_def testcases/case4/case4_cfg.json testcases/case4/case4.json
                python3 ./bin/draw_grid.py --input grid.csv --output grid4.png
                ./bin/CGR -draw 20 testcases/case5/case5_def testcases/case5/case5_cfg.json testcases/case5/case5.json
                python3 ./bin/draw_grid.py --input grid.csv --output grid5.png
            fi
        else
            ./bin/CGR -draw 20 testcases/case4/case4_def testcases/case4/case4_cfg.json testcases/case4/case4.json
            python3 ./bin/draw_grid.py --input grid.csv --output grid4.png
            ./bin/CGR -draw 20 testcases/case5/case5_def testcases/case5/case5_cfg.json testcases/case5/case5.json
            python3 ./bin/draw_grid.py --input grid.csv --output grid5.png
        fi
    elif [ $1 -eq "4" ]; then
        ./bin/CGR 20 testcases/case4/case4_def testcases/case4/case4_cfg.json testcases/case4/case4.json
    elif [ $1 -eq "5" ]; then
        ./bin/CGR 20 testcases/case5/case5_def testcases/case5/case5_cfg.json testcases/case5/case5.json
    fi
else
    ./bin/CGR 20 testcases/case4/case4_def testcases/case4/case4_cfg.json testcases/case4/case4.json
    ./bin/CGR 20 testcases/case5/case5_def testcases/case5/case5_cfg.json testcases/case5/case5.json
fi

rm -f grid.csv
#python3 ./bin/draw_route_large.py --grid ./grid.csv --lines ./case4_net.rpt --output route4.png
