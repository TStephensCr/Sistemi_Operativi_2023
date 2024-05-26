/*********************************P1TEST.C*******************************
 *
 *	Test program for the modules ASL and pcbQueues (phase 1).
 *
 *	Produces progress messages on terminal 0 in addition
 *		to the array ``okbuf[]''
 *		Error messages will also appear on terminal 0 in
 *		addition to the array ``errbuf[]''.
 *
 *		Aborts as soon as an error is detected.
 *
 *      Modified by Michael Goldweber on May 15, 2004
 */

#include "./headers/print.h"

char   okbuf[2048]; /* sequence of progress messages */
char   errbuf[128]; /* contains reason for failing */
char   msgbuf[128]; /* nonrecoverable error message before shut down */
pcb_t *procp[MAXPROC], *p, *q, *firstproc, *lastproc, *midproc;
char  *mp = okbuf;
msg_t *msg[MAXMESSAGES], *m, *n, *firstmsg, *lastmsg, *midmsg;


#define TRANSMITTED 5
#define ACK         1
#define PRINTCHR    2
#define CHAROFFSET  8
#define STATUSMASK  0xFF
#define TERM0ADDR   0x10000254

typedef unsigned int devreg;

/* This function returns the terminal transmitter status value given its address */
devreg termstat(memaddr *stataddr) {
    return ((*stataddr) & STATUSMASK);
}

/* This function prints a string on specified terminal and returns TRUE if
 * print was successful, FALSE if not   */
unsigned int termprint(char *str, unsigned int term) {
    memaddr     *statusp;
    memaddr     *commandp;
    devreg       stat;
    devreg       cmd;
    unsigned int error = FALSE;

    if (term < DEVPERINT) {
        /* terminal is correct */
        /* compute device register field addresses */
        statusp  = (devreg *)(TERM0ADDR + (term * DEVREGSIZE) + (TRANSTATUS * DEVREGLEN));
        commandp = (devreg *)(TERM0ADDR + (term * DEVREGSIZE) + (TRANCOMMAND * DEVREGLEN));

        /* test device status */
        stat = termstat(statusp);
        if (stat == READY || stat == TRANSMITTED) {
            /* device is available */

            /* print cycle */
            while (*str != EOS && !error) {
                cmd       = (*str << CHAROFFSET) | PRINTCHR;
                *commandp = cmd;

                /* busy waiting */
                stat = termstat(statusp);
                while (stat == BUSY)
                    stat = termstat(statusp);

                /* end of wait */
                if (stat != TRANSMITTED)
                    error = TRUE;
                else
                    /* move to next char */
                    str++;
            }
        } else
            /* device is not available */
            error = TRUE;
    } else
        /* wrong terminal device number */
        error = TRUE;

    return (!error);
}


/* This function placess the specified character string in okbuf and
 *	causes the string to be written out to terminal0 */
void debug(char *strp,int len, int b) {
    strp[len]= b+'0';
    strp[len+1]='\0';
    char *tstrp = strp;
    while ((*mp++ = *strp++) != '\0')
        ;
    mp--;
    termprint(tstrp, 0);
}
