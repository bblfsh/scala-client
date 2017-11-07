#ifdef __cplusplus
extern "C" {
#endif

#include "objtrack.h"

#include <jni.h>

#include <stdlib.h>

typedef struct {
  void **vector;
  size_t used;
  size_t size;
} AllocVector;

static AllocVector *_allocVector = NULL;

static void initAllocVector() {
  const int initialSize = 128;

  _allocVector = (AllocVector *)malloc(sizeof(AllocVector));
  _allocVector->vector = (void **)malloc(initialSize * sizeof(void*));
  _allocVector->used = 0;
  _allocVector->size = initialSize;
}

void trackObject(void *obj) {
  if (_allocVector == NULL || _allocVector->vector == NULL) {
    initAllocVector();
  }

  if (_allocVector->used == _allocVector->size) {
    _allocVector->size *= 2;
    _allocVector->vector = (void **)realloc(_allocVector->vector,
                           _allocVector->size * sizeof(void*));
  }
  _allocVector->vector[_allocVector->used++] = obj;
}

void freeObjects() {
  if (_allocVector == NULL)
    return; // No object ever tracked thus not initialized

  for (int i=0; i < _allocVector->used; i++) {
    free(_allocVector->vector[i]);
    _allocVector->vector[i] = NULL;
  }

  free(_allocVector->vector);
  _allocVector->vector = NULL;

  free(_allocVector);
  _allocVector = NULL;
}

#ifdef __cplusplus
}
#endif
