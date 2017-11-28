#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STAT_PROCESS_CURRENT	1
#define STAT_PROCESS_COMMENT	2

void process_linebuffer(char *linebuffer, int linecount)
{
    printf("line %d: %s\n", linecount, linebuffer);
}

int parser_parse(const char *filepath, 
		  const char *commleader, 
		  const char *commtailer)
{
    FILE *file = NULL;
    char linebuffer[128];
    char commbuffer[8];
    int linecount = 0;
    int state_current = STAT_PROCESS_CURRENT;
    int lenleader = 0;
    int lentailer = 0;
    int lb = 0;
    int lc = 0;
    int c  = '\0';
    int i;

    if (filepath == NULL || commleader == NULL || commtailer == NULL) {
	return -1;
    }

    file = fopen(filepath, "r");
    if (file == NULL) {
	fprintf(stderr, "open file failure\n");
	return -1;
    }

    lenleader = strlen(commleader);
    lentailer = strlen(commtailer);

    while ((c = fgetc(file)) != EOF) {

	switch (state_current) {
	    case STAT_PROCESS_CURRENT:

		/* encounter possible leader comment indicator */
		if (c == commleader[0]) {

		    /* read as many characters as @commleader into
		     * @commbuffer, then we can compare the two string to
		     * check whether the comment really starts 
		     */
		    commbuffer[lc++] = c;
		    for (; lc < lenleader; lc++) {
			commbuffer[lc] = fgetc(file);
		    }
		    commbuffer[lc] = '\0';
		    
		    /* check whether comment starts or not. 
		     *
		     * If we are in comment, change @state_current, then 
		     * find possible newline in commbuffer, if found, 
		     * process @linecount and @linebuffer properly. 
		     *
		     * If we are not in comment, then manually append 
		     * @commbuffer to @linebuffer, then process @linebuffer
		     * and @linecount properly according to '\n' found.
		     */
		    if (strcmp(commleader, commbuffer) == 0) {
			state_current = STAT_PROCESS_COMMENT;
			for (i = 0; i < lc; i++) {
			    if (commbuffer[i] == '\n') {
				linecount++;
				process_linebuffer(linebuffer, linecount);
				memset(linebuffer, '\0', lb);
				lb = 0;
			    }
			}
		    } else {
			for (i = 0; i < lc; i++) {
			    if (commbuffer[i] != '\n') {
				linebuffer[lb++] = commbuffer[i];
			    } else {
				linecount++;
				process_linebuffer(linebuffer, linecount);
				memset(linebuffer, '\0', lb);
				lb = 0;
			    }
			}
		    }

		    /* Once we find possible leader comment indicator, 
		     * after a series of operations, we must eventually
		     * clear @commbuffer and @lc for the next unrelated
		     * series of operations 
		     */
		    memset(commbuffer, '\0', lc);
		    lc = 0;
		} else {         /* just regular characters */
		    if (c != '\n') {
			linebuffer[lb++] = c;
		    } else {
			linecount++;
			linebuffer[lb] = '\0';
			process_linebuffer(linebuffer, linecount);
			memset(linebuffer, '\0', lb);
			lb = 0;
		    }
		}
		break;

	    case STAT_PROCESS_COMMENT:

		/* encounter possible tailer comment indicator */
		if (c == commtailer[0]) {

		    /* complete @commbuffer to compare with @commtailer */
		    commbuffer[lc++] = c;
		    for (; lc < lentailer; lc++) {
			commbuffer[lc] = fgetc(file);
		    }
		    commbuffer[lc] = '\0';

		    /* check if we reach the end of comment.
		     * If so, change status of @state_current, then process
		     * @linebuffer and @linecount properly according to all
		     * the '\n's found in @commbuffer
		     *
		     * If not, we do nothing
		     */
		    if (strcmp(commtailer, commbuffer) == 0) {
			state_current = STAT_PROCESS_CURRENT;
			for (i = 0; i < lc; i++) {
			    if (commbuffer[i] == '\n') {
				linecount++;
				process_linebuffer(linebuffer, linecount);
				memset(linebuffer, '\0', lb);
				lb = 0;
			    }
			}
		    }

		    /* Once operations related to @commbuffer are done,
		     * we must clear @commbuffer and @lc for next time.
		     */
		    memset(commbuffer, '\0', lc);
		    lc = 0;
		} else {        /* just regular comment characters */

		    /* if '\n' found in comment, we need increment the
		     * @linecount to keep tracking of line numbers, and
		     * @linebuffer may be empty or not, so we need to
		     * process @linebuffer too.
		     */
		    if (c == '\n') {
			linecount++;
			process_linebuffer(linebuffer, linecount);
			memset(linebuffer, '\0', lb);
			lb = 0;
		    }
		}
		break;

	    default: break;
	}
    }

    fclose(file);
    return 0;
}

int main(int argc, char *argv[])
{
    parser_parse(argv[1], "#", "\n");

    return 0;
}
