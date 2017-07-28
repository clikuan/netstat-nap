CC = gcc
PROGRAM_NAME = netstat-nap
main: main.c
	${CC} -o  ${PROGRAM_NAME} $^
clean:
	rm -f ${PROGRAM_NAME}
