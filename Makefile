.PHONY: all
all: compiler

compiler:
	cd src; \
	gcc -g -o ../ss *.c;

.PHONY: clean
clean:
	rm -f ss;
