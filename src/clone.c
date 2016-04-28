#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <sys/wait.h>
#include <stdbool.h>

#define STACKSIZE 1024*1024
static char child_stack[STACKSIZE];

struct clone_args {
  char **argv;
};

static int child_fn(void *stuff)
{
  struct clone_args *args = (struct clone_args *)stuff;

  if (sethostname("mycontainer", strlen("mycontainer")) != 0) {
    fprintf(stderr, "failed to set hostname to mycontainer. Did you create a new UTS namespace?\n");
    exit(1);
  }

  if (execvp(args->argv[0], args->argv) != 0) {
    fprintf(stderr, "Command failed");
    exit(1);
  }

  return -1;
}

int main(int argc, char *argv[])
{
  int flags = SIGCHLD;
  struct clone_args args;
  args.argv = &argv[1];

  int child_pid = clone(child_fn, child_stack + STACKSIZE, flags, &args);
  if (child_pid < 0) {
    fprintf(stderr, "OMG clone failed. The world is coming to end!!\n");
    return 1;
  }

  printf("Running child with PID: %d\n", child_pid);
  if (waitpid(child_pid, NULL, 0) == -1) {
    fprintf(stderr, "couldn't wait for child with PID: %d\n", child_pid);
    return 1;
  }
  return 0;
}
