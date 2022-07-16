files01 = out_case1.def ./cases/case1/case1.def ./cases/case1/case1.lef ./cases/case1/case1.txt
files02 = out_case2.def ./cases/case2/case2.def ./cases/case2/case2.lef ./cases/case2/case2.txt
files03 = out_case3.def ./cases/case3/case3.def ./cases/case3/case3.lef ./cases/case3/case3.txt
files04 = out_case4.def ./cases/case4/case4.def ./cases/case4/case4.lef ./cases/case4/case4.txt
files05 = out_case5.def ./cases/case5/case5.def ./cases/case5/case5.lef ./cases/case5/case5.txt
files06 = out_case6.def ./cases/case6/case6.def ./cases/case6/case6.lef ./cases/case6/case6.txt
files07 = out_case7.def ./cases/case7/case7.def ./cases/case7/case7.lef ./cases/case7/case7.txt
files08 = out_case8.def ./cases/case8/case8.def ./cases/case8/case8.lef ./cases/case8/case8.txt
files09 = out_case9.def ./cases/case9/case9.def ./cases/case9/case9.lef ./cases/case9/case9.txt
files10 = out_case10.def ./cases/case10/case10.def ./cases/case10/case10.lef ./cases/case10/case10.txt

src = ./lp_solve_5.5/lp_MDO.c ./lp_solve_5.5/shared/commonlib.c ./lp_solve_5.5/colamd/colamd.c ./lp_solve_5.5/shared/mmio.c ./lp_solve_5.5/shared/myblas.c ./lp_solve_5.5/ini.c ./lp_solve_5.5/lp_rlp.c ./lp_solve_5.5/lp_crash.c ./lp_solve_5.5/bfp/bfp_LUSOL/lp_LUSOL.c ./lp_solve_5.5/bfp/bfp_LUSOL/LUSOL/lusol.c ./lp_solve_5.5/lp_Hash.c ./lp_solve_5.5/lp_lib.c ./lp_solve_5.5/lp_wlp.c ./lp_solve_5.5/lp_matrix.c ./lp_solve_5.5/lp_mipbb.c ./lp_solve_5.5/lp_MPS.c ./lp_solve_5.5/lp_params.c ./lp_solve_5.5/lp_presolve.c ./lp_solve_5.5/lp_price.c ./lp_solve_5.5/lp_pricePSE.c ./lp_solve_5.5/lp_report.c ./lp_solve_5.5/lp_scale.c ./lp_solve_5.5/lp_simplex.c ./lp_solve_5.5/lp_SOS.c ./lp_solve_5.5/lp_utils.c ./lp_solve_5.5/yacc_read.c

cs_src = ./corner_stitch/tiles/tile.cpp ./corner_stitch/tiles/search.cpp ./corner_stitch/tiles/search2.cpp ./corner_stitch/utils/malloc.cpp ./corner_stitch/utils/update.cpp

c = /users/student/mr109/cwchiu20/gcc/gcc-10.1.0/bin/gcc

std = -lstdc++

math = -lm

# def = -dy -K PIC -DNOLONGLONG
dl = -ldl

opts = -O3

target = main.cpp handler.cpp macro.cpp graph.cpp constraint_graph.cpp adjustment.cpp compression.cpp evaluation.cpp strategies.cpp LP.cpp io.cpp transitive_reduction.cpp

executable = cadb0065


CC = g++

.DEFAULT_GOAL:  compile

compile:
	${c} ${std} -I./ -I./lp_solve_5.5 -I./lp_solve_5.5/bfp -I./lp_solve_5.5/bfp/bfp_LUSOL -I./lp_solve_5.5/bfp/bfp_LUSOL/LUSOL -I./lp_solve_5.5/colamd -I./lp_solve_5.5/shared -I./tr ${opts} -DYY_NEVER_INTERACTIVE -DPARSER_LP -DINVERSE_ACTIVE=INVERSE_LUSOL -DRoleIsExternalInvEngine -DSYSV -DHAVE_VA_COPY -pthread ${target} ${cs_src} ${src} -o ${executable} ${math} ${dl}

case01:
	./cadb0065 ${files01}
case02:
	./cadb0065 ${files02}
case03:
	./cadb0065 ${files03}
case04:
	./cadb0065 ${files04}
case05:
	./cadb0065 ${files05}
case06:
	./cadb0065 ${files06}
case07:
	./cadb0065 ${files07}
case08:
	./cadb0065 ${files08}
case09:
	./cadb0065 ${files09}
case10:
	./cadb0065 ${files10}