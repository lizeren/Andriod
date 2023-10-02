#include <jni.h>
#include <string>


//function to be called in java
extern "C" JNIEXPORT jstring JNICALL
Java_com_example_tempc_MainActivity_stringFromJNI(JNIEnv* env,jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
// use method from java
extern "C" JNIEXPORT jstring JNICALL Java_com_example_tempc_MainActivity_process(JNIEnv* env,jobject mainActivityInst) {
    const jclass mainActivityCls = env->GetObjectClass(mainActivityInst);
    const jmethodID methodid = env->GetMethodID(mainActivityCls,"processInJava","()Ljava/lang/String;");

    if(methodid == nullptr){
        return env->NewStringUTF("");
    }

    const jobject result = env->CallObjectMethod(mainActivityInst, methodid);
    const std::string java_msg = env->GetStringUTFChars((jstring) result, JNI_FALSE);
    const std::string c_msg = "result from Java =>";
    const std::string msg = c_msg + java_msg;

    return env->NewStringUTF(msg.c_str());
}