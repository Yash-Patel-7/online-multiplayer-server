all: cleanExec ttts ttt test cleanDSYM

clean: cleanExec cleanDSYM

ttts:
	gcc -g -Wall -Werror -fsanitize=address -std=c99 ttts.c helper.c net.c -o ttts -pthread

ttt:
	gcc -g -Wall -Werror -fsanitize=address -std=c99 ttt.c helper.c net.c -o ttt -pthread

test:
	gcc -g -Wall -Werror -fsanitize=address -std=c99 test.c helper.c net.c -o test -pthread

cleanExec:
	rm -rf ttts && rm -rf ttt && rm -rf test

cleanDSYM:
	rm -rf ttts.dSYM && rm -rf ttt.dSYM && rm -rf test.dSYM
