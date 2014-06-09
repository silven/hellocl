CFLAGS=-g -O0 -Wall 
LFLAGS=-lOpenCL -lrt
OUT=hello

all: $(OUT)

.PHONY: clean

clean:
	$(RM) $(OUT)
	$(RM) *.o

$(OUT): hellocl.c
	clang -std=c11 -ggdb $(CFLAGS) $< -o $@ $(LFLAGS)

