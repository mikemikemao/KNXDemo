//
// Created by pirate
//
#include <com/ComFunc.h>
#include <com/Define.h>
#include "jni.h"
#include "utils/LogUtil.h"
#define NATIVE_CLASS_NAME "com/hikvision/jni/IKNX"

/*
* Class:     com_hikvision_jni_IKNX
* Method:    openLight
* Signature: ()V
*/
JNIEXPORT void JNICALL openLight(JNIEnv *env, jobject instance)
{
    int ret = 0;
    ret = comLightControl(1);
	if (ret != ERR_OK)
	{
		LOGCATE("comLightControl failed ret =%d",ret);
	}
	else{
		LOGCATE("comLightControl success");
	}
}

//视频预览相关
static JNINativeMethod g_methods[] = {
        {"native_openLight",                      "()V",       (void *)(openLight)},
};

static int RegisterNativeMethods(JNIEnv *env, const char *className, JNINativeMethod *methods, int methodNum)
{
	LOGCATE("RegisterNativeMethods");
	jclass clazz = env->FindClass(className);
	if (clazz == NULL)
	{
		LOGCATE("RegisterNativeMethods fail. clazz == NULL");
		return JNI_FALSE;
	}
	if (env->RegisterNatives(clazz, methods, methodNum) < 0)
	{
		LOGCATE("RegisterNativeMethods fail");
		return JNI_FALSE;
	}
	return JNI_TRUE;
}

static void UnregisterNativeMethods(JNIEnv *env, const char *className)
{
	LOGCATE("UnregisterNativeMethods");
	jclass clazz = env->FindClass(className);
	if (clazz == NULL)
	{
		LOGCATE("UnregisterNativeMethods fail. clazz == NULL");
		return;
	}
	if (env != NULL)
	{
		env->UnregisterNatives(clazz);
	}
}

// call this func when loading lib
extern "C" jint JNI_OnLoad(JavaVM *jvm, void *p)
{
	LOGCATE("===== JNI_OnLoad =====");
	jint jniRet = JNI_ERR;
	JNIEnv *env = NULL;
	if (jvm->GetEnv((void **) (&env), JNI_VERSION_1_6) != JNI_OK)
	{
		return jniRet;
	}
	//视频预览相关操作
	jint regRet = RegisterNativeMethods(env, NATIVE_CLASS_NAME, g_methods,
										sizeof(g_methods) /
										sizeof(g_methods[0]));
	if (regRet != JNI_TRUE)
	{
		return JNI_ERR;
	}

	return JNI_VERSION_1_6;
}

extern "C" void JNI_OnUnload(JavaVM *jvm, void *p)
{
	JNIEnv *env = NULL;
	if (jvm->GetEnv((void **) (&env), JNI_VERSION_1_6) != JNI_OK)
	{
		return;
	}
	UnregisterNativeMethods(env, NATIVE_CLASS_NAME);
}