#ifdef __cplusplus
extern "C" {
#endif

#include "allocvector.h"

#include <jni.h>

#include <stdlib.h>

void initAllocVector(AllocVector *l) {
  int initialSize = 128;

  l->vector = (jobject **)malloc(initialSize * sizeof(jobject*));
  l->used = 0;
  l->size = initialSize;
}

void trackAllocatedJObject(AllocVector *l, jobject *obj) {
  if (l->used == l->size) {
    l->size *= 2;
    l->vector = (jobject **)realloc(l->vector, l->size * sizeof(jobject*));
  }
  l->vector[l->used++] = obj;
}

void freeAllocVector(AllocVector *l) {
  int i;
  for (i=0; i < l->used; i++) {
    free(l->vector[i]);
    l->vector[i] = NULL;
  }

  free(l->vector);
  l->vector = NULL;
  l->used = l->size = 0;
}

#ifdef __cplusplus
}
#endif
