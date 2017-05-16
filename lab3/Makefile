# Wesley Makcey's Makefile for string_set with slight modifications
# Juan Gonzalez 1497521
# Cmps 104a

GPP      = g++ -g -O0 -Wall -Wextra -std=gnu++14   
MKDEPS   = g++ -std=gnu++14 -MM

MKFILE   = Makefile
DEPSFILE = Makefile.deps
SOURCES  = auxlib.cpp stringset.cpp main.cpp lyutils.cpp astree.cpp
HEADERS  = auxlib.h stringset.h
OBJECTS  = ${SOURCES:.cpp=.o} ${CLGEN:.cc=.o} ${CYGEN:.cc=.o}
EXECBIN  = oc
SRCFILES = ${HEADERS} ${SOURCES} ${MKFILE}

LSOURCES = scanner.l
YSOURCES = parser.y
CLGEN    = yylex.cc
HYGEN    = yyparse.h
CYGEN    = yyparse.cc
LREPORT  = yylex.output
YREPORT  = yyparse.output

all : ${HYGEN} ${EXECBIN}

${EXECBIN} : ${OBJECTS}
	${GPP} ${OBJECTS} -o ${EXECBIN}

%.o : %.cpp
	${GPP} -c $<

ci :
	cid + ${SRCFILES}

clean :
	-rm ${OBJECTS} ${DEPSFILE} ${CLGEN} ${HYGEN} ${CYGEN} \
	${LREPORT} ${YREPORT}

spotless : clean
	- rm ${EXECBIN}

deepclean: spotless
	- rm *.str *.tok *.ast *.err *.out *.oc *.lexyacctrace

${CLGEN} : ${LSOURCES}
	flex --outfile=${CLGEN} ${LSOURCES} 2>${LREPORT}
	- grep -v '^ ' ${LREPORT}

${CYGEN} ${HYGEN} : ${YSOURCES}
	bison --defines=${HYGEN} --output=${CYGEN} ${YSOURCES}

${DEPSFILE} :
	${MKDEPS} ${SOURCES} ${CLGEN} ${CYGEN} >>${DEPSFILE}

deps :
	-rm ${DEPSFILE}
	${MAKE} --no-print-directory ${DEPSFILE}
