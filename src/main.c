#include <stdio.h>
#include <esp_attr.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "fonts.h"
#include "graphics.h"

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
    char str[64];
    vTaskDelay(1);
    tv[0]=tv[1]=tv[2]=tv[3]=tv[4]=tv[5]=1;
    // get time since boot in microseconds
    uint64_t time=esp_timer_get_time();
    unsigned ccount,icount,ccount_new;
    RSR(XT_REG_CCOUNT,ccount);
    WSR(XT_REG_ICOUNT, 0);
    WSR(XT_REG_ICOUNTLEVEL,2);
    fn();
    RSR(XT_REG_CCOUNT,ccount_new);
    RSR(XT_REG_ICOUNT,icount);
    time=esp_timer_get_time()-time;
    float cpi=(float)(ccount_new-ccount)/icount;
    setFontColour(255,255,255);
    snprintf(str,64,"%s",name);
    print_xy(str,0,LASTY+8);
    setFontColour(0,255,0);
    snprintf(str,64,"%.2f MOP/S",(float)N/time);
    print_xy(str,22*5,LASTY);
    setFontColour(0,255,255);
    snprintf(str,64,"CPI:%.2f",cpi);
    print_xy(str,37*5,LASTY);
    printf ("%22s %.2f MOP/S CPI=%.2f\n",name, (float)N/time,cpi);
    
}
extern int antialias;
void app_main() {
    // initialise graphics and lcd display
    graphics_init();
    setFont(FONT_DEJAVU24);
    setFontColour(32,255,32);
    cls(0);
    gprintf("Running Benchmark");
    flip_frame();
    setFont(FONT_SMALL);
    setFontColour(255,255,255);
    while(1) {
        cls(0);
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
        flip_frame();
    }
}
