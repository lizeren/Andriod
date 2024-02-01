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

//#define pmccntr
#define cntvct_el0
uint64_t get_monotonic_time(void)
{
    struct timespec t1;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    return t1.tv_sec * 1000*1000*1000ULL + t1.tv_nsec;
}

inline uint64_t perf_get_timing(libflush_session_t* session)
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


    // timer


    uint64_t temp_disp1,temp_disp2,diff;
    uint64_t freq;



    // cache flushing
//    uint8_t array[10*8192];
//    int temp,temp2;
//    int i;
//    // Initialize the array
//    for(i=0; i<10; i++) array[i*8192]=1;
//
//    // FLUSH the array from the CPU cache
//    for(i=0; i<10; i++) {
//        asm volatile ("dc civac, %0" : : "r" (&array[i*8192]) : "memory");
//    }
//
//    // Access some of the array items
//    array[7*8192] = 200;
//
//    int64_t fast_diff1, fast_diff2;
//    asm volatile("MRS %0, cntvct_el0 " : "=r" (fast_diff1));
//    temp2 = array[7*8192];
//    asm volatile("MRS %0, cntvct_el0 " : "=r" (fast_diff2));
//    const std::string fast_diff = std::to_string(fast_diff2 - fast_diff1);
//
//
//    int64_t slow_diff1, slow_diff2;
//    asm volatile("MRS %0, cntvct_el0 " : "=r" (slow_diff1));
//    temp = array[1*8192];
//    asm volatile("MRS %0, cntvct_el0 " : "=r" (slow_diff2));
//    const std::string slow_diff = std::to_string(slow_diff2 - slow_diff1);
#if defined(pmccntr)
    arm_v8_timing_init();
    uint64_t result_pmccntr = arm_v8_get_timing();
    asm volatile ("isb"); // Serialize after reading the counter
#endif
#if defined(cntvct_el0)
    asm volatile("MRS %0, cntfrq_el0 " : "=r" (freq));
    asm volatile("MRS %0, cntvct_el0 " : "=r" (temp_disp1));
    asm volatile ("isb"); // Serialize after reading the counter
#endif


    uint8_t array[256 * 8192];
    volatile uint8_t *volatile_array = array; // Declare a volatile pointer to the array
    int temp, temp2, i;
    int64_t fast_diff1, fast_diff2;
    int64_t slow_diff1, slow_diff2;

// Initialize the array
    for (i = 0; i < 256; i++) {
        volatile_array[i * 8192] = 1;
        asm volatile ("dsb ish"); // Data synchronization barrier ensure write has completed
    }

// Access some of the array items
    volatile_array[7 * 8192] = 200;


// Timestamp and array access with serialization and data barriers
    asm volatile ("isb"); // Serialize before reading the counter
    asm volatile ("MRS %0, cntvct_el0" : "=r" (fast_diff1));
    asm volatile ("isb"); // Serialize after reading the counter
    temp2 = volatile_array[7 * 8192];
    asm volatile ("dsb ish"); // Data synchronization barrier after volatile access
    asm volatile ("isb"); // Serialize before reading the counter again
    asm volatile ("MRS %0, cntvct_el0" : "=r" (fast_diff2));
    asm volatile ("isb"); // Serialize after reading the counter
    const std::string fast_diff = std::to_string(fast_diff2 - fast_diff1);


    // FLUSH the array from the CPU cache
    for (i = 0; i < 256; i++) {
        asm volatile ("dc civac, %0" : : "r" (&volatile_array[i * 8192]) : "memory");
        //asm volatile ("dsb ish"); // Data synchronization barrier ensure invalidation has completed
        asm volatile ("isb"); // Insert isb for serialization after cache flush
    }


// Similar pattern for the second measurement
    asm volatile ("isb");
    asm volatile ("MRS %0, cntvct_el0" : "=r" (slow_diff1));
    asm volatile ("isb");
    temp = volatile_array[1 * 8192];
    asm volatile ("dsb ish");
    asm volatile ("isb");
    asm volatile ("MRS %0, cntvct_el0" : "=r" (slow_diff2));
    asm volatile ("isb");
    const std::string slow_diff = std::to_string(slow_diff2 - slow_diff1);

    const std::string unoptmized = std::to_string(temp - temp2);

#if defined(pmccntr)
    uint64_t result_pmccntr2 = arm_v8_get_timing();
    asm volatile ("isb"); // Serialize after reading the counter
    const std::string show_result_pmccntr = std::to_string(result_pmccntr2 - result_pmccntr);
    const std::string temp_disp_string = "no temp_disp_string";
    const std::string show1 = "no show1";
    const std::string show2 = "no show2";
    const std::string show_freq = "no show_freq";

#endif
#if defined(cntvct_el0)
    asm volatile("MRS %0, cntvct_el0 " : "=r" (temp_disp2));
    asm volatile ("isb"); // Serialize after reading the counter
    diff = temp_disp2 - temp_disp1;
    const std::string temp_disp_string = std::to_string(diff);
    const std::string show1 = std::to_string(temp_disp1);
    const std::string show2 = std::to_string(temp_disp2);
    const std::string show_freq = std::to_string(freq);
    const std::string show_result_pmccntr = "no result_pmccntr";
#endif








    std::string msg = c_msg + java_msg  +
                            "\ndiff value:"+ temp_disp_string +
                            "\none:" + show1 +
                            "\ntwo:" + show2 +
                            "\nfreq:" + show_freq +
                            "\nresult_pmccntr:" + show_result_pmccntr +
                            "\nslow:" + slow_diff +
                            "\nfast:" + fast_diff ;
    return env->NewStringUTF(msg.c_str());
}