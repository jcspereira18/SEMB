#include <pthread.h>
#include <sched.h>
#include <stdio.h>

#include "include/components/init_comp.hpp"
#include "include/threads.hpp"

void measureExecutionTime(void *(*threadFunc)(void *), void *args,
                          const char *threadName) {
  struct timespec start, end;
  double elapsedTime, maxExecutionTime = 0.0;

  for (int i = 1; i <= 30; i++) { // Repeat the thread execution to find WCET
    // Record the start time with high precision
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_t thread;
    if (pthread_create(&thread, NULL, threadFunc, args) != 0) {
      printf("[ERROR] - Cannot create %s\n", threadName);
      exit(EXIT_FAILURE);
    }
    pthread_join(thread, NULL);

    // Record the end time with high precision
    clock_gettime(CLOCK_MONOTONIC, &end);
    // Calculate elapsed time in microseconds with finer precision
    elapsedTime =
        (end.tv_sec - start.tv_sec) * 1e6 + (end.tv_nsec - start.tv_nsec) / 1e3;

    // Update the maximum execution time if needed
    if (elapsedTime > maxExecutionTime) {
      maxExecutionTime = elapsedTime;
    }

  /* printf("[CYCLE] WCET for %s: %.3f microseconds in iteration %d\n", threadName, i);   */
  }

  // Print the worst-case execution time with finer precision
  printf("[INFO] WCET for %s: %.3f microseconds\n", threadName,
         maxExecutionTime);
}

int main() {
  CubeSystem c; // System struct

  // Scheduliing attributes
  pthread_attr_t initAttributes;
  pthread_attr_t displayAttr;
  pthread_attr_t RRthreads;

  pthread_attr_init(&initAttributes);
  pthread_attr_init(&displayAttr);
  pthread_attr_init(&RRthreads);

  pthread_attr_setschedpolicy(&initAttributes, SCHED_FIFO);
  pthread_attr_setschedpolicy(&displayAttr, SCHED_RR);
  pthread_attr_setschedpolicy(&RRthreads, SCHED_RR);

  struct sched_param initParam;
  struct sched_param displayParam;
  struct sched_param RRthreadsParam;

  initParam.sched_priority = 3; // Higher means higher prio.
  displayParam.sched_priority = 2;
  RRthreadsParam.sched_priority = 1;

  pthread_attr_setschedparam(&initAttributes, &initParam);
  pthread_attr_setschedparam(&displayAttr, &displayParam);
  pthread_attr_setschedparam(&RRthreads, &RRthreadsParam);

  pthread_attr_setinheritsched(&initAttributes, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setinheritsched(&displayAttr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setinheritsched(&RRthreads, PTHREAD_EXPLICIT_SCHED);

  measureExecutionTime(createCubeSystem, &c, "createCubeSystem");
  measureExecutionTime(displayCube, &c, "displayCube");
  measureExecutionTime(readButtons, &c, "readButtons");
  measureExecutionTime(updateSnakeDirection, &c, "updateSnakeDirection");
  measureExecutionTime(systemStateTransitions, &c, "systemStateTransitions");
  measureExecutionTime(systemStateActions, &c, "systemStateActions");

  // reset expanders and shifters to exit program
  pthread_t resetThread;
  if (pthread_create(&resetThread, NULL, globalReset, (void *)&c) != 0) {
    printf("[ERROR] - Cannot create resetThread\n");
    exit(EXIT_FAILURE);
  }
  pthread_join(resetThread, NULL);

  pthread_attr_destroy(&initAttributes);
  pthread_attr_destroy(&RRthreads);

  return EXIT_SUCCESS;
}

