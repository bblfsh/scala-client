#ifdef __cplusplus
extern "C" {
#endif

#include "alloclist.h"

#include <jni.h>

#include <stdlib.h>

void initAllocList(AllocList *l, size_t initialSize) {
  l->list = (jobject **)malloc(initialSize * sizeof(jobject*));
  l->used = 0;
  l->size = initialSize;
}

void trackAllocatedJObject(AllocList *l, jobject *obj) {
  if (l->used == l->size) {
    l->size *= 2;
    l->list = (jobject **)realloc(l->list, l->size * sizeof(int));
  }
  l->list[l->used++] = obj;
}

void freeAllocList(AllocList *l) {
  int i;
  for (i=0; i < l->used; i++) {
    free(l->list[i]);
  }

  free(l->list);
  l->list = NULL;
  l->used = l->size = 0;
}

#ifdef __cplusplus
}
#endif
