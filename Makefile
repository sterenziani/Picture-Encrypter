.PHONY: all
all: compiler

compiler:
	cd src; \
	gcc -o ../ss *.c;

.PHONY: clean
clean:
	rm -f ss;