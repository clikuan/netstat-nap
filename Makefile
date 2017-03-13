CC = gcc
PROGRAM_NAME = netstat-like-program
permission: main
	sudo chown root.root ${PROGRAM_NAME}
	sudo chmod +s ${PROGRAM_NAME}
main: main.c
	${CC} -o  ${PROGRAM_NAME} $^

clean:
	rm -f ${PROGRAM_NAME}
