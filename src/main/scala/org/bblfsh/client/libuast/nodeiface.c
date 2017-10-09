#ifdef __cplusplus
extern "C" {
#endif

#include "nodeiface.h"

#include <stdint.h>
#include <jni.h>

extern JNIEnv *env;

static const char *InternalType(const void *node)
{
  return ReadStr((const jobject*)node, "internalType");
}

static const char *Token(const void *node)
{
  return ReadStr((jobject*)node, "token");
}

static int ChildrenSize(const void *node)
{
  return ReadLen((jobject*)node, "children");
}

static int RolesSize(const void *node)
{
  return ReadLen((jobject*)node, "roles");
}

static void *ChildAt(const void *data, int index)
{
  jobject *node = (jobject *)data;
  jobject childSeq = ObjectField(CLS_NODE, node, "children", SIGN_SEQ);
  if ((*env)->ExceptionOccurred(env))
    return NULL;

  jobject child = ObjectMethod("apply", SIGN_APPLY, CLS_SEQ, childSeq, index);
  if ((*env)->ExceptionOccurred(env) || !child)
    return NULL;

  return ToObjectPtr(&child);
}

static int PropertiesSize(const void *data)
{
  jobject *node = (jobject *)data;
  jobject propsMap = ObjectField(CLS_NODE, node, "properties", SIGN_MAP);
  if ((*env)->ExceptionOccurred(env) || !propsMap)
    return 0;

  return (int)IntMethod("size", "()I", CLS_MAP, &propsMap);
}

static const char *PropertyAt(const void *data, int index)
{
  jobject *node = (jobject *)data;
  jobject propsMap = ObjectField(CLS_NODE, node, "properties", SIGN_MAP);
  if ((*env)->ExceptionOccurred(env) || !propsMap)
    return NULL;

  // Convert to List
  jobject list = ObjectMethod("toList", SIGN_TOLIST, CLS_MAP, propsMap);
  if ((*env)->ExceptionOccurred(env) || !list)
    return NULL;

  // Get the key/value tuple at the "index" position. The tuple is given as a Seq.
  jobject kvTuple = ObjectMethod("apply", SIGN_APPLY, CLS_SEQ, list, index);
  if ((*env)->ExceptionOccurred(env) || !kvTuple)
    return NULL;

  // Get the "_1" field and convert to char*
  jobject key = ObjectField(CLS_TUPLE2, &kvTuple, "_1", SIGN_OBJECT);
  if ((*env)->ExceptionOccurred(env) || !key)
    return NULL;

  return AsNativeStr((jstring)key);
}

static uint16_t RoleAt(const void *data, int index)
{
  jobject *node = (jobject *)data;
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

Uast *CreateUast()
{
  Uast *ctx = UastNew((NodeIface){
      .InternalType = InternalType,
      .Token = Token,
      .ChildrenSize = ChildrenSize,
      .ChildAt = ChildAt,
      .RolesSize = RolesSize,
      .RoleAt = RoleAt,
      .PropertiesSize = PropertiesSize,
      .PropertyAt = PropertyAt
  });
}


#ifdef __cplusplus
}
#endif
