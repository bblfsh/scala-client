#include "nodeiface.h"
#include "jni_utils.h"

#include <stdbool.h>
#include <stdint.h>
#include <jni.h>

//// Interface Helpers
typedef enum {KEY, VALUE} kv;

static const jstring _PropertyItemAt(const void *_node, int index, kv item) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return NULL;

  jobject *node = (jobject *)_node;
  jobject propsMap = ObjectField(env, CLS_NODE, node, "properties", TYPE_MAP);
  if (env->ExceptionOccurred() || !propsMap)
    return NULL;

  // get the key at index pos: Node.properties.keys.toList.sorted(index):
  jobject iterable = ObjectMethod(env, "keys", METHOD_MAP_KEYS, CLS_MAP, &propsMap);
  if (env->ExceptionOccurred() || !iterable)
    return NULL;

  jobject keyList = ObjectMethod(env, "toList", METHOD_MUTLIST_TOLIST, CLS_ITERABLE, &iterable);
  if (env->ExceptionOccurred() || !keyList)
    return NULL;

  jobject sortedKeyList = ObjectMethod(env, "sorted", METHOD_SEQ_SORTED, CLS_SEQ, &keyList, NULL);
  if (env->ExceptionOccurred() || !sortedKeyList)
    return NULL;

  jobject key = ObjectMethod(env, "apply", METHOD_SEQ_APPLY, CLS_SEQ, &sortedKeyList, index);
  if (env->ExceptionOccurred() || !key)
    return NULL;

  if (item == KEY) {
    return (jstring)key;
  }

  // else: get the value: Node.properties.get(key).get
  jobject value = ObjectMethod(env, "apply", METHOD_MAP_APPLY, CLS_MAP, &propsMap, key);
  if (env->ExceptionOccurred() || !value)
    return NULL;

  return (jstring)value;
}

static jobject _GetPositionObject(const void *_node, const char *posname) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return NULL;

  jobject *node = (jobject *)_node;

  jobject positionOption = ObjectField(env, CLS_NODE, node, posname, TYPE_OPTION);
  if (env->ExceptionOccurred() || !positionOption)
    return NULL;

  jboolean isEmpty = BooleanMethod(env, "isEmpty", "()Z", CLS_OPTION, &positionOption);
  if (env->ExceptionOccurred() || (bool)isEmpty)
    return NULL;

  jobject position = ObjectMethod(env, "get", METHOD_OPTION_GET, CLS_OPTION, &positionOption);
  if (env->ExceptionOccurred() || !position)
    return NULL;

  return position;
}

static bool HasPosition(const void *node, const char *posname) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return false;

  jobject pos = _GetPositionObject(node, posname);
  return pos != NULL && !env->ExceptionOccurred();
}

static uint32_t _SubPositionValue(const void *node, const char *posname, const char *subpos) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return false;

  jobject pos = _GetPositionObject(node, posname);
  if (env->ExceptionOccurred() || !pos)
    return 0;

  jint value = IntField(env, CLS_POSITION, &pos, subpos);
  return env->ExceptionOccurred() ? 0 : (uint32_t)value;
}


//// Interface functions
static const char *InternalType(const void *node) {
  return ReadStr((const jobject*)node, "internalType");
}

static const char *Token(const void *node) {
  return ReadStr((jobject*)node, "token");
}

static size_t ChildrenSize(const void *node) {
  return ReadLen((jobject*)node, "children");
}

static size_t RolesSize(const void *node) {
  return ReadLen((jobject*)node, "roles");
}

static void *ChildAt(const void *_node, int index) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return NULL;

  jobject *node = (jobject *)_node;
  jobject childSeq = ObjectField(env, CLS_NODE, node, "children", TYPE_SEQ);
  if (env->ExceptionOccurred())
    return NULL;

  jobject child = ObjectMethod(env, "apply", METHOD_SEQ_APPLY, CLS_SEQ, &childSeq, index);
  if (env->ExceptionOccurred() || !child)
    return NULL;

  return ToObjectPtr(&child);
}

static size_t PropertiesSize(const void *_node) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return 0;

  jobject *node = (jobject *)_node;
  jobject propsMap = ObjectField(env, CLS_NODE, node, "properties", TYPE_MAP);
  if (env->ExceptionOccurred() || !propsMap)
    return 0;

  return (int)IntMethod(env, "size", "()I", CLS_MAP, &propsMap);
}

static const char *PropertyKeyAt(const void *node, int index) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return NULL;

  jstring str = _PropertyItemAt(node, index, KEY);
  if (env->ExceptionOccurred() || !str)
    return NULL;

  return AsNativeStr(str);
}

static const char *PropertyValueAt(const void *node, int index) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return NULL;

  jstring str = _PropertyItemAt(node, index, VALUE);
  if (env->ExceptionOccurred() || !str)
    return NULL;

  return AsNativeStr(str);
}

static uint16_t RoleAt(const void *_node, int index) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return 0;

  jobject *node = (jobject *)_node;
  jobject roleSeq = ObjectField(env, CLS_NODE, node, "roles", TYPE_SEQ);
  if (env->ExceptionOccurred() || !roleSeq)
    return 0;

  jobject roleObj = ObjectMethod(env, "apply", METHOD_SEQ_APPLY, CLS_SEQ, &roleSeq, index);
  if (env->ExceptionOccurred() || !roleObj)
    return 0;

  jint roleNum = IntMethod(env, "value", "()I", CLS_ROLE, &roleObj);
  if (env->ExceptionOccurred())
    return 0;

  return (uint16_t)roleNum;
}

static bool HasStartPosition(const void *node) {
  return HasPosition(node, "startPosition");
}

static bool HasEndPosition(const void *node) {
  return HasPosition(node, "endPosition");
}

static uint32_t StartOffset(const void *node) {
  return _SubPositionValue(node, "startPosition", "offset");
}

static uint32_t StartLine(const void *node) {
  return _SubPositionValue(node, "startPosition", "line");
}

static uint32_t StartCol(const void *node) {
  return _SubPositionValue(node, "startPosition", "col");
}

static uint32_t EndOffset(const void *node) {
  return _SubPositionValue(node, "endPosition", "offset");
}

static uint32_t EndLine(const void *node) {
  return _SubPositionValue(node, "endPosition", "line");
}

static uint32_t EndCol(const void *node) {
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
