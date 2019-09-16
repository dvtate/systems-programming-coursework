#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/wait.h>

int main () {
  int i, bg, pid;
  char buf[100];

  /* read-eval loop */
  while (!feof(stdin)) {
    printf(">");  /* print prompt */

    /* read command and remove newline */
    fgets(buf, 100, stdin);
    for (i=strlen(buf)-1; buf[i]=='\n'; buf[i--]=0);

    bg = (buf[i] == '&');
    if (bg) 
      buf[i] = 0;

    if (strcmp(buf, "quit") == 0) {
      break;
    } else {
      /* fork and run command in child */
      if ((pid = fork()) == 0) {
        if (execlp(buf, buf, (char *)0) < 0) {
          printf("Command not found\n");
          exit(0);
        }
      }
    }

    if (!bg) {
      /* wait for completion in parent */
      waitpid(pid, NULL, 0);
    }
  }
  return 0;
}
