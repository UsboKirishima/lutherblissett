SRC := $(foreach x, ./src, $(wildcard $(addprefix $(x)/*,.c*)))
LIB := $(foreach x, ./lib, $(wildcard $(addprefix $(x)/*,.c*)))
DIRS := $(foreach x, ./src/**, $(wildcard $(addprefix $(x)/*,.c*)))
out.o: 
	gcc ${SRC} ${DIRS} -I include ${INCLUDES}  -o build/lb  2> /dev/null && ./build/lb

test: 
	gcc ${SRC} ${DIRS} -I include ${INCLUDES}  -o build/lb  && ./build/lb
