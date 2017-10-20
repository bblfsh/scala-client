#ifdef __cplusplus
extern "C" {
#endif

#include "allocvector.h"

#include <jni.h>

#include <stdlib.h>

// XXX
#include <stdio.h>

struct _AllocVector{
  jobject **vector;
  size_t used;
  size_t size;
};

AllocVector *allocVector;

static void initAllocVector(AllocVector **vPtr) {
  int initialSize = 128;

  *vPtr = (AllocVector *)malloc(sizeof(AllocVector));
  *vPtr->vector = (jobject **)malloc(initialSize * sizeof(jobject*));
  *vPtr->used = 0;
  *vPtr->size = initialSize;
}

void trackAllocatedJObject(AllocVector *v, jobject *obj) {
  printf("XXX 1\n");
  if (v == NULL || v->vector == NULL) {
    printf("XXX 2\n");
    initAllocVector();
  }

  printf("XXX 3\n");
  if (v->used == v->size) {
    printf("XXX 4\n");
    v->size *= 2;
    printf("XXX 5\n");
    v->vector = (jobject **)realloc(v->vector, v->size * sizeof(jobject*));
    printf("XXX 6\n");
  }
  printf("XXX 7\n");
  v->vector[v->used++] = obj;
  printf("XXX 8\n");
}

void freeAllocVector(AllocVector **vPtr) {
  int i;
  for (i=0; i < *vPtr->used; i++) {
    free(*vPtr->vector[i]);
    *vPtr->vector[i] = NULL;
  }

  free(*vPtr->vector);
  *vPtr->vector = NULL;

  free(*vPtr);
  *vPtr = NULL;
}

#ifdef __cplusplus
}
#endif
