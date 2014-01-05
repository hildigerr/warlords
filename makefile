# $Id: makefile,v 1.18 2013/12/09 19:40:13 moonsdad Exp $

CC = gcc
CFLAGS = -g -Wall
CCONF = `pkg-config --cflags --libs gtk+-2.0`
SLINK = -lpcre -lncurses

BASE_SRC = rmhv_stdlib.c game.c
BASE_HED = ${BASE_SRC:.c=.h} game.h
SRV_SRCS = ${BASE_SRC} servsig.c cardplay.c sgui.c server.c
CLI_SRCS = ${BASE_SRC} cligui.c clibuf.c warlord_ai.c clichan.c warlords.c

all : server client

server : ${SRV_SRCS}
	${CC} ${CFLAGS} ${SRV_SRCS} ${SLINK} -o warlords-${@}

client : ${CLI_SRCS}
	${CC} ${CFLAGS} ${CLI_SRCS} -o warlords-${@} ${CCONF}

man: warlords.6
	gzip -c warlords.6 > warlords.6.gz

clean:
	rm warlords-server warlords-client

${SRV_SRCS} : ${BASE_HED} sgui.h server.h
${CLI_SRCS} : ${BASE_HED} client.h warlord_ai.h
