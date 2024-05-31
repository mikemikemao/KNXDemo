//
// Created by pirate
//
#include <com/ComFunc.h>
#include <com/Define.h>
#include <knx/KNXWrapper.h>
#include <test/UnitTest.h>
#include <zltoolkit/Util/logger.h>
#include "jni.h"
#include "knxcore/KnxFunc.h"
#define NATIVE_CLASS_NAME "com/hikvision/jni/IKNX"
using namespace toolkit;

/*
* Class:     com_hikvision_jni_IKNX
* Method:    openLight
* Signature: ()V
*/
JNIEXPORT void JNICALL openLight(JNIEnv *env, jobject instance,jint lightState)
{
    int ret = 0;
    //ret =  recvService();
    //ret = comLightStateControl(lightState);
	ret = comLightControl(lightState);
    //ret = KnxlightControl(lightState);
    //ret = knxTest();
	if (ret != ERR_OK)
	{
		LogE("comLightControl failed ret =%d",ret);
	}
	else{
		LogE("comLightControl success");
	}
}

JNIEXPORT int JNICALL openLightState(JNIEnv *env, jobject instance)
{
	LogE("openLightState");
	return comLightStateControl();
}

JNIEXPORT void JNICALL test(JNIEnv *env, jobject instance)
{
	 //LoggerTest();
	eventPollerTest();
}

//视频预览相关
static JNINativeMethod g_methods[] = {
        {"native_openLight",                      "(I)V",       (void *)(openLight)},
		{"native_openLightState",                 "()I",        (void *)(openLightState)},
		{"native_test",                           "()V",        (void *)(test)},
};

static int RegisterNativeMethods(JNIEnv *env, const char *className, JNINativeMethod *methods, int methodNum)
{
	LogE("RegisterNativeMethods");
	jclass clazz = env->FindClass(className);
	if (clazz == NULL)
	{
		LogE("RegisterNativeMethods fail. clazz == NULL");
		return JNI_FALSE;
	}
	if (env->RegisterNatives(clazz, methods, methodNum) < 0)
	{
		LogE("RegisterNativeMethods fail");
		return JNI_FALSE;
	}
	return JNI_TRUE;
}

static void UnregisterNativeMethods(JNIEnv *env, const char *className)
{
	LogE("UnregisterNativeMethods");
	jclass clazz = env->FindClass(className);
	if (clazz == NULL)
	{
		LogE("UnregisterNativeMethods fail. clazz == NULL");
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
	LogE("===== JNI_OnLoad =====");
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