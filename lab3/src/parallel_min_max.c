#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed < 0) {
              printf("Seed must be non-negative\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("Array size must be positive\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
              printf("Pnum must be positive\n");
              return 1;
            }
            break;
          case 3:
            with_files = true;
            break;

          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  //pipes creation
  
  int (*pipefd)[2] = NULL;
  if (!with_files) {
    pipefd = malloc(sizeof(int[2]) * pnum);
    if (pipefd == NULL) {
      perror("malloc");
      free(array);
      return 1;
    }
    for (int i = 0; i < pnum; i++) {
      if (pipe(pipefd[i]) == -1) {
        perror("pipe");
        // clean up 
        for (int j = 0; j < i; j++) {
          close(pipefd[j][0]);
          close(pipefd[j][1]);
        }
        free(pipefd);
        free(array);
        return 1;
      }
    }
  }

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      active_child_processes += 1;
      if (child_pid == 0) {

        int start = i * (array_size / pnum);
        int end = (i == pnum - 1) ? array_size : start + (array_size / pnum);

        int local_min = INT_MAX;
        int local_max = INT_MIN;
        for (int k = start; k < end; k++) {
          if (array[k] < local_min) local_min = array[k];
          if (array[k] > local_max) local_max = array[k];
        }

        if (with_files) { //files
          char filename[256];
          snprintf(filename, sizeof(filename), "tmp_minmax_%d.txt", i);
          FILE *f = fopen(filename, "w");
          if (f == NULL) {
            perror("fopen");
            // Even on error, exit child
            return 1;
          }
          fprintf(f, "%d %d\n", local_min, local_max);
          fclose(f);
        } else { //pipe
          // close read end in child
          close(pipefd[i][0]); 
          // write two integers
          if (write(pipefd[i][1], &local_min, sizeof(local_min)) == -1) {
            perror("write min");
            // attempt to close and exit
            close(pipefd[i][1]);
            return 1;
          }
          if (write(pipefd[i][1], &local_max, sizeof(local_max)) == -1) {
            perror("write max");
            close(pipefd[i][1]);
            return 1;
          }
          // close write end after writing
          close(pipefd[i][1]);
        }
        return 0;
      } else {
        // parent process
        if (!with_files) {
          // parent doesn't need write end
          close(pipefd[i][1]);
        }
      }

    } else {
      printf("Fork failed!\n");
      // cleanup pipes if allocated
      if (pipefd) {
        for (int j = 0; j < pnum; j++) {
          close(pipefd[j][0]);
          close(pipefd[j][1]);
        }
        free(pipefd);
      }
      free(array);
      return 1;
    }
  }

  while (active_child_processes > 0) {
    // your code here
    wait(NULL);
    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      // read from files
      char filename[256];
      snprintf(filename, sizeof(filename), "tmp_minmax_%d.txt", i);
      FILE *f = fopen(filename, "r");
      if (f == NULL) {
        perror("fopen read");
        // treat as error: continue
      } else {
        if (fscanf(f, "%d %d", &min, &max) != 2) {
          // reading failed
          perror("fscanf");
        }
        fclose(f);
        remove(filename);
      }
    } else {
      // read from pipes
      // read two integers from read end
      ssize_t r = read(pipefd[i][0], &min, sizeof(min));
      if (r == -1) {
        perror("read min");
      } else if (r == 0) {
        // no data
      }
      r = read(pipefd[i][0], &max, sizeof(max));
      if (r == -1) {
        perror("read max");
      } else if (r == 0) {
        // no data
      }
      close(pipefd[i][0]);
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  if (pipefd) free(pipefd);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
