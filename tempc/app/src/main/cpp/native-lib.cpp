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

uint8_t array[10*4096];


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
/*
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
            + "\nPerf timer diff:"+ time_string_perf +
            "\ntemp value:"+ temp_disp_string;
*/


    arm_v8_timing_init();
    int64_t result_pmccntr = arm_v8_get_timing();

    int64_t temp_disp1,temp_disp2,diff;
    int32_t freq;
    asm volatile("MRS %0, cntvct_el0 " : "=r" (temp_disp1));
    asm volatile("MRS %0, cntvct_el0 " : "=r" (temp_disp2));
    asm volatile("MRS %0, cntfrq_el0 " : "=r" (freq));

    diff = temp_disp2 - temp_disp1;
    const std::string temp_disp_string = std::to_string(diff);
    const std::string show1 = std::to_string(temp_disp1);
    const std::string show2 = std::to_string(temp_disp2);
    const std::string show_freq = std::to_string(freq);
    const std::string show_result_pmccntr = std::to_string(result_pmccntr);



    int temp,temp2;
    array[3*4096] = 100;
    array[7*4096] = 200;

    int64_t slow_diff1, slow_diff2;
    asm volatile("MRS %0, cntvct_el0 " : "=r" (slow_diff1));
    temp = array[1*4096];
    asm volatile("MRS %0, cntvct_el0 " : "=r" (slow_diff2));
    const std::string slow_diff = std::to_string(slow_diff2 - slow_diff1);


    int i;
    for(i=0; i<10; i++) array[i*4096]=1;
    for(i=0; i<10; i++) {
        asm volatile ("dc civac, %0" : : "r" (array) : "memory");
    }


    int64_t fast_diff1, fast_diff2;
    asm volatile("MRS %0, cntvct_el0 " : "=r" (fast_diff1));
    temp2 = array[3*4096];
    asm volatile("MRS %0, cntvct_el0 " : "=r" (fast_diff2));
    const std::string fast_diff = std::to_string(fast_diff2 - fast_diff1);

    const std::string msg = c_msg + java_msg  +
                            "\ndiff value:"+ temp_disp_string +
                            "\none:" + show1 +
                            "\ntwo:" + show2 +
                            "\nfreq:" + show_freq +
                            "\nresult_pmccntr:" + show_result_pmccntr +
                            "\nslow:" + slow_diff +
                            "\nfast:" + fast_diff ;

    // Use libflush...
    // Terminate libflush
    //libflush_terminate(libflush_session);


	return env->NewStringUTF(msg.c_str());
}