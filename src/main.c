#include <stdio.h>
#include <esp_attr.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static int tv[6];
const int N = 3200000;

 
#define TEST(type,name,ops) void IRAM_ATTR name (void) {\
    type f0 = (type)tv[0],f1 = (type)tv[1],f2 = tv[2],f3 = tv[3];\
    type f4 = tv[4],f5 = tv[5]; \
    for (int j = (N/12); j > 0; j--) {\
        ops \
    }\
    tv[0] = f0;tv[1] = f1;tv[2] = f2;tv[3] = f3;\
    tv[4] = f4;tv[5] = f5;\
    }
    
#define fops(op1,op2) f0 op1##=f1 op2 f2;f1 op1##=f2 op2 f3;\
    f2 op1##=f3 op2 f4;f3 op1##=f4 op2 f5;\
    f4 op1##=f5 op2 f0;f5 op1##=f0 op2 f1;

#define addops fops(,+) fops(,+)
#define divops fops(,/) fops(,/)
#define mulops fops(,*) fops(,*)
#define muladdops fops(+,*)

TEST(int,mulint,mulops)
TEST(float,mulfloat,mulops)
TEST(double,muldouble,mulops)
TEST(int,addint,addops)
TEST(float,addfloat,addops)
TEST(double,adddouble,addops)
TEST(int,divint,divops)
TEST(float,divfloat,divops)
TEST(double,divdouble,divops)
TEST(int,muladdint,muladdops)
TEST(float,muladdfloat,muladdops)
TEST(double,muladddouble,muladdops)

void timeit(char *name,void fn(void)) {
    vTaskDelay(1);
    tv[0]=tv[1]=tv[2]=tv[3]=tv[4]=tv[5]=1;
    // get time since boot in microseconds
    uint64_t time=esp_timer_get_time();
    unsigned ccount,icount,ccount_new;
    RSR(CCOUNT,ccount);
    WSR(ICOUNT, 0);
    WSR(ICOUNTLEVEL,2);
    fn();
    RSR(CCOUNT,ccount_new);
    RSR(ICOUNT,icount);
    time=esp_timer_get_time()-time;
    float cpi=(float)(ccount_new-ccount)/icount;
    printf ("%s \t %f MOP/S   \tCPI=%f\n",name, (float)N/time,cpi);
}

void app_main() {
    while(1) {
        timeit("Integer Addition",addint);
        timeit("Integer Multiply",mulint);
        timeit("Integer Division",divint);
        timeit("Integer Multiply-Add",muladdint);

        timeit("Float Addition ", addfloat);
        timeit("Float Multiply ", mulfloat);
        timeit("Float Division ", divfloat);
        timeit("Float Multiply-Add", muladdfloat);

        timeit("Double Addition", adddouble);
        timeit("Double Multiply", muldouble);
        timeit("Double Division", divdouble);
        timeit("Double Multiply-Add", muladddouble);
    }
}
