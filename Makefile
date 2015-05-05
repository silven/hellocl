CFLAGS=-ggdb -O3 -Wall -Wextra
LFLAGS=-lOpenCL -lrt
OUT=hello

all: $(OUT)

.PHONY: clean

clean:
	$(RM) $(OUT)
	$(RM) *.o

$(OUT): hellocl.c Makefile
	clang -std=c11 $(CFLAGS) $< -o $@ $(LFLAGS)

