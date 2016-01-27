
CFLAGS	         =
FFLAGS	         =
CPPFLAGS         =
FPPFLAGS         =
LOCDIR           = src/ksp/ksp/examples/tutorials/
EXAMPLESC        = ex1.c ex2.c ex3.c ex4.c ex5.c ex7.c ex8.c ex9.c \
                ex10.c ex11.c ex12.c ex13.c ex15.c ex16.c ex18.c ex23.c \
                ex25.c ex27.c ex28.c ex29.c ex30.c ex31.c ex32.c ex34.c \
                ex41.c ex42.c ex43.c \
                ex45.c ex46.c  ex49.c ex50.c ex51.c ex52.c ex53.c \
                ex54.c ex55.c ex56.c ex58.c ex62.c ex63.cxx \
		playground.cpp
EXAMPLESF        = ex1f.F ex2f.F ex6f.F ex11f.F ex13f90.F ex14f.F ex15f.F ex21f.F ex22f.F ex44f.F90 ex45f.F \
                   ex52f.F ex54f.F ex61f.F90
MANSEC           = KSP
CLEANFILES       = rhs.vtk solution.vtk
NP               = 1

include ${PETSC_DIR}/lib/petsc/conf/variables
include ${PETSC_DIR}/lib/petsc/conf/rules

play: 
	mpiexec -np 4 ./playground

cl:
	${RM} playground.o dataProcess.o

playground: cl playground.o dataProcess.o chkopts
	-${CLINKER} -o playground playground.o dataProcess.o  ${PETSC_KSP_LIB}


	


ex1: ex1.o  chkopts
	-${CLINKER} -o ex1 ex1.o  ${PETSC_KSP_LIB}
	${RM} ex1.o

ex2: ex2.o  chkopts
	-${CLINKER} -o ex2 ex2.o  ${PETSC_KSP_LIB}
	${RM} ex2.o

include ${PETSC_DIR}/lib/petsc/conf/test
