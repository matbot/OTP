#Compiler and Flags
CC = gcc
CFLAGS = -std=c99 -g3 -pedantic -Wall -O2

#Sources
SRCS = *.c

#Headers
HEADERS = 
ifdef *.h
	*.h
endif

#Objects
OBJS = *.o

#Documentation
DOCS = *.pdf

#Programs
PROG = prog

#Compressed File
TAR = cs.tar.bz2

#####################################################
# BUILD and TAR
# ###################################################

all: keygen ed ec dd dc

keygen: keygen.c 
	${CC} ${CFLAGS} keygen.c -o keygen
ed: otp_enc_d.c otp_utils.c otp_utils.h
	${CC} ${CFLAGS} otp_enc_d.c otp_utils.c -o otp_enc_d
ec: otp_enc.c otp_utils.c otp_utils.h
	${CC} ${CFLAGS} otp_enc.c otp_utils.c -o otp_enc
dd: otp_dec_d.c otp_utils.c otp_utils.h
	${CC} ${CFLAGS} otp_dec_d.c otp_utils.c -o otp_dec_d
dc: otp_dec.c otp_utils.c otp_utils.h
	${CC} ${CFLAGS} otp_dec.c otp_utils.c -o otp_dec


prog: ${OBJS} ${HEADERS}
	${CC} ${CFLAGS} ${OBJS} -o ${PROG}

${OBJS}: ${SRCS}
	${CC} ${CFLAGS} -c ${@:.o=.c}

tar:
	tar cvjf ${TAR} ${SRCS} ${HEADERS} ${DOCS} makefile

###################
#CLEAN
###################

clean:
	rm -f ${PROG} *.o *~
