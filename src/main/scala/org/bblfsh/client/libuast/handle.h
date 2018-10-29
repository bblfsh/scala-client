#ifndef _HANDLE_H_INCLUDED_
#define _HANDLE_H_INCLUDED_

jfieldID getHandleField(JNIEnv *env, jobject obj, const char *name) {
    jclass cls = env->GetObjectClass(obj);
    if (env->ExceptionOccurred() || !cls) {
        return NULL;
    }

    jfieldID jfid = env->GetFieldID(cls, name, "J");
    if (env->ExceptionOccurred() || !jfid) {
        return NULL;
    }
    return jfid;
}

template <typename T>
T *getHandle(JNIEnv *env, jobject obj, const char *name) {
    jlong handle = env->GetLongField(obj, getHandleField(env, obj, name));
    if (env->ExceptionOccurred() || !handle) {
        return NULL;
    }
    return reinterpret_cast<T *>(handle);
}

template <typename T>
void setHandle(JNIEnv *env, jobject obj, T *t, const char *name) {
    jlong handle = reinterpret_cast<jlong>(t);
    env->SetLongField(obj, getHandleField(env, obj, name), handle);
}

#endif
