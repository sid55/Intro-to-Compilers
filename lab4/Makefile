CPPSOURCES = oc.cpp auxlib.cpp string_set.cpp astree.cpp lyutils.cpp   \
             symbol_table.cpp
HEADERS = auxlib.h string_set.h astree.h lyutils.h symbol_table.h
ALLSOURCES = ${CPPSOURCES} ${HEADERS} ${LSOURCES} ${YSOURCES} Makefile \
             README PARTNER
OBJECTS = ${CPPSOURCES:%.cpp=%.o} yylex.o yyparse.o
LSOURCES = scanner.l
YSOURCES = parser.y
CLGEN = yylex.cpp
HYGEN = yyparse.h
CYGEN = yyparse.cpp
LREPORT = yylex.output
YREPORT = yyparse.output
COMPILE=g++ -g -O0 -Wall -Wextra -std=gnu++14
EXCBIN=oc

all: ${EXCBIN}

${EXCBIN}: ${CLGEN} ${CYGEN} ${HYGEN} ${OBJECTS}
	-checksource ${CPPSOURCES} ${HEADERS} ${LSOURCES} \
	${YSOURCES} Makefile README
	${COMPILE} -o $@ ${OBJECTS}

%.o: %.cpp
	${COMPILE} -c $<

${CLGEN} : ${LSOURCES}
	flex --outfile=${CLGEN} ${LSOURCES} 2>${LREPORT}
	-grep -v '^ ' ${LREPORT}

${CYGEN} ${HYGEN} : ${YSOURCES}
	bison --defines=${HYGEN} --output=${CYGEN} ${YSOURCES}

ci:
	git add -A
	git commit -m "Regular file saving"
	git push

clean:
	-rm ${OBJECTS} ${CLGEN} ${HYGEN} ${CYGEN} ${LREPORT} ${YREPORT}

spotless:
	-rm ${OBJECTS} ${CLGEN} ${HYGEN} ${CYGEN} ${LREPORT} \
   ${YREPORT} ${EXCBIN}

submit:
	-checksource ${CPPSOURCES} ${HEADERS} ${LSOURCES} \
	${YSOURCES} Makefile README
	-submit cmps104a-wm.f16 asg4 ${CPPSOURCES} ${HEADERS} \
   ${LSOURCES} ${YSOURCES} Makefile README PARTNER
