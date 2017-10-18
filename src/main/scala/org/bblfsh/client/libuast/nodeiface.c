#ifdef __cplusplus
extern "C" {
#endif

#include "nodeiface.h"

#include <stdbool.h>
#include <stdint.h>
#include <jni.h>

// XXX remove
#include <stdio.h>

// XXX review types

//// Helpers
static const char *TUPLEKEY = "_1";
static const char *TUPLEVAL = "_2";

// XXX FIXME SORT (call .sorted on the List to get a new list)
static const jstring _PropertyItemAt(const void *_node, int index, const char *tupleitem) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return NULL;

  jobject *node = (jobject *)_node;
  jobject propsMap = ObjectField(CLS_NODE, node, "properties", SIGN_MAP);
  if ((*env)->ExceptionOccurred(env) || !propsMap)
    return NULL;

  // Get the keys
  jobject iterable = ObjectMethod("keys", SIGN_KEYS, CLS_MAP, propsMap);
  if ((*env)->ExceptionOccurred(env) || !iterable)
    return NULL;

  // Convert the iterable to list
  jobject keyList = ObjectMethod("toList", SIGN_TOLIST, CLS_ITERABLE, iterable);
  if ((*env)->ExceptionOccurred(env) || !keyList)
    return NULL;

  // Sort the list into a new list
  jobject sortedKeyList = ObjectMethod("sorted", SIGN_SORTED, CLS_SEQ, keyList, NULL);
  if ((*env)->ExceptionOccurred(env) || !sortedKeyList)
    return NULL;

  // XXX NEXT:


  // Convert the map to a List of tuples
  jobject list = ObjectMethod("toList", SIGN_TOLIST, CLS_MAP, propsMap);
  if ((*env)->ExceptionOccurred(env) || !list)
    return NULL;

  printf("XXX 1\n");
  // Get the sorted List
  jobject sortedList = ObjectMethod("sorted", SIGN_SORTED, CLS_SEQ, list, NULL);
  if ((*env)->ExceptionOccurred(env) || !sortedList)
    return NULL;
  printf("XXX 2\n");

  // Get the key/value tuple at the "index" position. The tuple is given as a Seq.
  jobject kvTuple = ObjectMethod("apply", SIGN_APPLY, CLS_SEQ, list, index);
  if ((*env)->ExceptionOccurred(env) || !kvTuple)
    return NULL;

  // Get the _1 or _2 field and convert to char*
  jobject item = ObjectField(CLS_TUPLE2, &kvTuple, tupleitem, SIGN_OBJECT);
  if ((*env)->ExceptionOccurred(env) || !item)
    return NULL;

  return (jstring)item;
}

jobject _GetPositionObject(const void *_node, const char *posname) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return NULL;

  jobject *node = (jobject *)_node;

  jobject positionOption = ObjectField(CLS_NODE, node, posname, SIGN_OPTION);
  if ((*env)->ExceptionOccurred(env) || !positionOption)
    return NULL;

  jboolean isEmpty = BooleanMethod("isEmpty", "()Z", CLS_OPTION, &positionOption);
  if ((*env)->ExceptionOccurred(env) || (bool)isEmpty)
    return NULL;

  jobject position = ObjectMethod("get", SIGN_OPTION_GET, CLS_OPTION, positionOption);
  if ((*env)->ExceptionOccurred(env) || !position)
    return NULL;

  return position;
}

// XXX add static
bool _HasPosition(const void *node, const char *posname) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return false;

  jobject pos = _GetPositionObject(node, posname);
  return pos != NULL && !(*env)->ExceptionOccurred(env);
}

static uint32_t _SubPositionValue(const void *node, const char *posname, const char *subpos) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return false;

  jobject pos = _GetPositionObject(node, posname);
  if ((*env)->ExceptionOccurred(env) || !pos)
    return 0;

  jint value = IntField(CLS_POSITION, &pos, subpos);
  return (*env)->ExceptionOccurred(env) ? 0 : (uint32_t)value;
}


//// Interface functions
static const char *InternalType(const void *node) {
  return ReadStr((const jobject*)node, "internalType");
}

static const char *Token(const void *node) {
  return ReadStr((jobject*)node, "token");
}

static int ChildrenSize(const void *node) {
  return ReadLen((jobject*)node, "children");
}

static int RolesSize(const void *node) {
  return ReadLen((jobject*)node, "roles");
}

static void *ChildAt(const void *_node, int index) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return NULL;

  jobject *node = (jobject *)_node;
  jobject childSeq = ObjectField(CLS_NODE, node, "children", SIGN_SEQ);
  if ((*env)->ExceptionOccurred(env))
    return NULL;

  jobject child = ObjectMethod("apply", SIGN_APPLY, CLS_SEQ, childSeq, index);
  if ((*env)->ExceptionOccurred(env) || !child)
    return NULL;

  return ToObjectPtr(&child);
}

static int PropertiesSize(const void *_node) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return 0;

  jobject *node = (jobject *)_node;
  jobject propsMap = ObjectField(CLS_NODE, node, "properties", SIGN_MAP);
  if ((*env)->ExceptionOccurred(env) || !propsMap)
    return 0;

  return (int)IntMethod("size", "()I", CLS_MAP, &propsMap);
}

// XXX add static
const char *PropertyKeyAt(const void *node, int index) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return NULL;

  jstring str = _PropertyItemAt(node, index, TUPLEKEY);
  if ((*env)->ExceptionOccurred(env) || !str)
    return NULL;

  return AsNativeStr(str);
}

// XXX add static
const char *PropertyValueAt(const void *node, int index) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return NULL;

  jstring str = _PropertyItemAt(node, index, TUPLEVAL);
  if ((*env)->ExceptionOccurred(env) || !str)
    return NULL;

  return AsNativeStr(str);
}

static uint16_t RoleAt(const void *_node, int index) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return 0;

  jobject *node = (jobject *)_node;
  jobject roleSeq = ObjectField(CLS_NODE, node, "roles", SIGN_SEQ);
  if ((*env)->ExceptionOccurred(env) || !roleSeq)
    return 0;

  jobject roleObj = ObjectMethod("apply", SIGN_APPLY, CLS_SEQ, roleSeq, index);
  if ((*env)->ExceptionOccurred(env) || !roleObj)
    return 0;

  jint roleNum = IntMethod("value", "()I", CLS_ROLE, &roleObj);
  if ((*env)->ExceptionOccurred(env))
    return 0;

  return (uint16_t)roleNum;
}

// XXX add static
bool HasStartPosition(const void *node) {
  return _HasPosition(node, "startPosition");
}

// XXX add static
bool HasEndPosition(const void *node) {
  return _HasPosition(node, "endPosition");
}

// XXX add static
uint32_t StartOffset(const void *node) {
  return _SubPositionValue(node, "startPosition", "offset");
}

// XXX add static
uint32_t StartLine(const void *node) {
  return _SubPositionValue(node, "startPosition", "line");
}

// XXX add static
uint32_t StartCol(const void *node) {
  return _SubPositionValue(node, "startPosition", "col");
}

// XXX add static
uint32_t EndOffset(const void *node) {
  return _SubPositionValue(node, "endPosition", "offset");
}

// XXX add static
uint32_t EndLine(const void *node) {
  return _SubPositionValue(node, "endPosition", "line");
}

// XXX add static
uint32_t EndCol(const void *node) {
  return _SubPositionValue(node, "endPosition", "col");
}
Uast *CreateUast() {
  return UastNew((NodeIface){
    .InternalType = InternalType,
    .Token = Token,
    .ChildrenSize = ChildrenSize,
    .ChildAt = ChildAt,
    .RolesSize = RolesSize,
    .RoleAt = RoleAt,
    .PropertiesSize = PropertiesSize,
    .PropertyKeyAt = PropertyKeyAt,
    .PropertyValueAt = PropertyValueAt,
    .HasStartOffset = HasStartPosition,
    .StartOffset = StartOffset,
    .HasStartLine = HasStartPosition,
    .StartLine = StartLine,
    .HasStartCol = HasStartPosition,
    .StartCol = StartCol,
    .HasEndOffset = HasEndPosition,
    .EndOffset = EndOffset,
    .HasEndLine = HasEndPosition,
    .EndLine = EndLine,
    .HasEndCol = HasEndPosition,
    .EndCol = EndCol
  });
}


#ifdef __cplusplus
}
#endif
