#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

int main (int argc, char *argv[])
{
    FILE *fp;
    
    openlog(NULL,0,LOG_USER);
    
    if (argc != 3) {
        syslog(LOG_ERR, "Usage: %s <filename> <text>", argv[0]);
        return 1;
    }

    fp = fopen(argv[1], "w");
    if (fp == NULL) {
        syslog(LOG_ERR, "failed to open %s for writing", argv[1]);
        fclose(fp);
        return 1;
    }

    syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
    fprintf(fp, "%s", argv[2]);
    fclose(fp);
    return 0;
}
