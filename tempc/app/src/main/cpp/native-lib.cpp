#include <jni.h>
#include <string>
#include <unistd.h>
#include "helper.h"
#include "include/armv8inc/libflush.h"
#include "include/armv8inc/internal.h"
#include "include/armv8inc/memory.h"
#include "include/armv8inc/timing.h"
#include "include/internal.h"
#include "include/libflush.h"
#include "include/timing.h"
#include <time.h>

//function to be called in java

uint64_t
get_monotonic_time(void)
{
    struct timespec t1;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    return t1.tv_sec * 1000*1000*1000ULL + t1.tv_nsec;
}

inline uint64_t
perf_get_timing(libflush_session_t* session)
{
    long long result = 0;

    if (read(session->perf.fd, &result, sizeof(result)) < (ssize_t) sizeof(result)) {
        return 0;
    }

    return result;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_tempc_MainActivity_stringFromJNI(JNIEnv* env,jobject /* this */) {
	std::string hello = "Hello from pure C++ accessed by Java";
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

    //test();

    libflush_session_t* libflush_session;
    if (libflush_init(&libflush_session, NULL) == false) {

    }
    uint64_t time1_mono = get_monotonic_time();
    //sleep(1);
    uint64_t time2_mono = get_monotonic_time();
    uint64_t diff_mono = time2_mono - time1_mono;
    const std::string time_string_mono = std::to_string(diff_mono);


    uint64_t time1_perf = perf_get_timing(libflush_session);
    uint64_t time2_perf = perf_get_timing(libflush_session);
    uint64_t diff_perf = time2_perf - time1_perf;
    const std::string time_string_perf = std::to_string(diff_perf);


    const std::string msg = c_msg + java_msg
            + "\nMonotonic timer diff:" +time_string_mono +
            + "\nPerf timer diff:"+ time_string_perf ;
    // Use libflush...
    // Terminate libflush
    //libflush_terminate(libflush_session);


	return env->NewStringUTF(msg.c_str());
}