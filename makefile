# $Id: makefile,v 1.19 2014/02/05 21:53:19 hildigerr Exp $

CC = gcc
CFLAGS = -g -Wall -std=gnu89
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
	@ gzip -c warlords.6 > warlords.6.gz

install: all man
	@ mv warlords-client ${DESTDIR}/usr/bin/
	@ mv warlords-server ${DESTDIR}/usr/bin/
	@ cp auto_cli.exp ${DESTDIR}/usr/bin/warlords-auto
	@ mv warlords.6.gz ${DESTDIR}/usr/share/man/man6/
	@ mv 13031717912126086382istockphoto_6922629-wild-joker-in-a-deck-of-cards-th.png \
	  ${DESTDIR}/usr/share/icons/warscum.png

${SRV_SRCS} : ${BASE_HED} sgui.h server.h
${CLI_SRCS} : ${BASE_HED} client.h warlord_ai.h
