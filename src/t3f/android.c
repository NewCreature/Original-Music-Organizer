#include "t3f.h"

/* The code below is from a supposedly working implementation. We should derive
   our implementation from it and see if we can actually find our
   MainActivity class. */

#ifdef ALLEGRO_ANDROID

	#include <jni.h>

	#define JNI_FUNC_PASTER(ret, cls, name, params, x) \
		JNIEXPORT ret JNICALL Java_ ## x ## _ ## cls ## _ ## name params
	#define JNI_FUNC_EVALUATOR(ret, cls, name, params, x) \
		JNI_FUNC_PASTER(ret, cls, name, params, x)
	#define JNI_FUNC(ret, cls, name, params) \
		JNI_FUNC_EVALUATOR(ret, cls, name, params, T3F_ANDROID_NATIVE_CALL_PREFIX)

	JNIEnv *_al_android_get_jnienv();
	void __jni_checkException(JNIEnv *env, const char *file, const char *fname, int line);
	jobject _al_android_activity_object();

	#define _jni_checkException(env) __jni_checkException(env, __FILE__, __FUNCTION__, __LINE__)

	#define _jni_call(env, rett, method, args...) ({ \
	   rett ret = (*env)->method(env, args); \
	   _jni_checkException(env); \
	   ret; \
	})

	#define _jni_callv(env, method, args...) ({ \
	   (*env)->method(env, args); \
	   _jni_checkException(env); \
	})

	#define _jni_callVoidMethodV(env, obj, name, sig, args...) ({ \
	   jclass class_id = _jni_call(env, jclass, GetObjectClass, obj); \
	   \
	   jmethodID method_id = _jni_call(env, jmethodID, GetMethodID, class_id, name, sig); \
	   if(method_id == NULL) { \
	   } else { \
		  _jni_callv(env, CallVoidMethod, obj, method_id, ##args); \
	   } \
	   \
	   _jni_callv(env, DeleteLocalRef, class_id); \
	})

	#define _jni_callBooleanMethodV(env, obj, name, sig, args...) ({ \
	   jclass class_id = _jni_call(env, jclass, GetObjectClass, obj); \
	   \
	   jmethodID method_id = _jni_call(env, jmethodID, GetMethodID, class_id, name, sig); \
	   \
	   bool ret = false; \
	   if(method_id == NULL) { \
	   } \
	   else { \
		  ret = _jni_call(env, bool, CallBooleanMethod, obj, method_id, ##args); \
	   } \
	   \
	   _jni_callv(env, DeleteLocalRef, class_id); \
	   \
	   ret; \
	})

	#define _jni_callIntMethodV(env, obj, name, sig, args...) ({ \
	   jclass class_id = _jni_call(env, jclass, GetObjectClass, obj); \
	   \
	   jmethodID method_id = _jni_call(env, jmethodID, GetMethodID, class_id, name, sig); \
	   \
	   int ret = -1; \
	   if(method_id == NULL) { \
		  /*ALLEGRO_DEBUG("couldn't find method :(");*/ \
	   } \
	   else { \
		  ret = _jni_call(env, int, CallIntMethod, obj, method_id, ##args); \
	   } \
	   \
	   _jni_callv(env, DeleteLocalRef, class_id); \
	   \
	   ret; \
	})

	#define _jni_callIntMethod(env, obj, name) _jni_callIntMethodV(env, obj, name, "()I");

	static jobject _jni_callObjectMethod(JNIEnv *env, jobject object, const char *name, const char *sig)
	{
	   jclass class_id = _jni_call(env, jclass, GetObjectClass, object);
	   jmethodID method_id = _jni_call(env, jmethodID, GetMethodID, class_id, name, sig);
	   jobject ret = _jni_call(env, jobject, CallObjectMethod, object, method_id);
	   _jni_callv(env, DeleteLocalRef, class_id);

	   return ret;
	}

	static char * t3f_edit_box_text = NULL; // pointer to text currently being edited in edit box
	static int t3f_edit_box_text_size = 0;
	static void(*t3f_edit_box_callback)(void * data) = NULL;
	static void(*t3f_edit_box_staged_callback)(void * data) = NULL;
	static void * t3f_edit_box_data = NULL;

JNI_FUNC(void, MainActivity, nativeOnEditComplete, (JNIEnv *env, jobject obj, jstring returnedS))
	{
		(void)obj;

		const char * s;

		s = (*env)->GetStringUTFChars(env, returnedS, 0);
		if(s)
		{
			memcpy(t3f_edit_box_text, s, t3f_edit_box_text_size);
			(*env)->ReleaseStringUTFChars(env, returnedS, s);
		}
		t3f_edit_box_callback = t3f_edit_box_staged_callback;
		t3f_edit_box_staged_callback = NULL;
	}

	/* Should be called every logic tick so we can react to calls from Java in
	   our main thread. We need to do this in case some callback messes with
	   OpenGL textures or some other thing which must be done on the thread that
	   has the context. */
	void t3f_android_support_helper(void)
	{
		if(t3f_edit_box_callback)
		{
			t3f_edit_box_callback(t3f_edit_box_data);
			t3f_edit_box_callback = NULL;
			t3f_edit_box_data = NULL;
		}
	}

	void t3f_open_edit_box(const char * title, char * text, int text_size, const char * flags, void(*callback)(void * data), void * data)
	{
		t3f_edit_box_text = text; // point
		t3f_edit_box_text_size = text_size;
		t3f_edit_box_staged_callback = callback;
		t3f_edit_box_data = data;
		JNIEnv * env = _al_android_get_jnienv();
		jstring titleS = (*env)->NewStringUTF(env, title);
		jstring initialS = (*env)->NewStringUTF(env, text);
		jstring flagsS = (*env)->NewStringUTF(env, flags);

		_jni_callVoidMethodV(
			_al_android_get_jnienv(),
			_al_android_activity_object(),
			"OpenEditBox",
			"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
			titleS,
			initialS,
			flagsS
		);
		(*env)->DeleteLocalRef(env, titleS);
		(*env)->DeleteLocalRef(env, initialS);
	}

	void t3f_show_soft_keyboard(bool toggle)
	{
        if(toggle)
        {
			_jni_callVoidMethodV(_al_android_get_jnienv(), _al_android_activity_object(), "OpenKeyBoard", "()V");
		}
		else
		{
			_jni_callVoidMethodV(_al_android_get_jnienv(), _al_android_activity_object(), "CloseKeyBoard", "()V");
		}
	}

	void t3f_open_url(const char *url)
	{
		JNIEnv * env = _al_android_get_jnienv();
		jstring urlS = (*env)->NewStringUTF(env, url);

		_jni_callVoidMethodV(
			_al_android_get_jnienv(),
			_al_android_activity_object(),
			"openURL",
			"(Ljava/lang/String;)V",
			urlS
		);
		(*env)->DeleteLocalRef(env, urlS);
	}

#else

	void t3f_android_support_helper(void)
	{
	}

	void t3f_open_edit_box(const char * title, char * text, int text_size, const char * flags, void(*callback)(void * data), void * data)
	{
	}

	void t3f_show_soft_keyboard(bool toggle)
	{
	}

	void t3f_open_url(const char *url)
	{
		char command[256] = {0};

		#ifdef ALLEGRO_WINDOWS
			snprintf(command, 256, "start %s", url);
		#else
			#ifdef ALLEGRO_MACOSX
				snprintf(command, 256, "open %s", url);
			#else
				snprintf(command, 256, "xdg-open %s", url);
			#endif
		#endif
		al_stop_timer(t3f_timer);
		(void) system(command);
		al_start_timer(t3f_timer);
	}
#endif
