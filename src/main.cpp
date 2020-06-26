#include <cstdio>
#include <chrono>
#include <array>
#include <functional>
#include "esp_attr.h"



int N = 1600000;

const int M = 3;

static double test_values[4];

template <typename T>
[[gnu::noinline, gnu::optimize ("fast-math")]]
void IRAM_ATTR test_addition (void)
{
  T f0 = test_values[0];
  T f1 = test_values[1];
  T f2 = test_values[2];
  T f3 = test_values[3];

  for (int j = 0; j < N/16; j++)
  {
    f0 += f3;
    f1 += f2;
    f2 += f1;
    f3 += f0;
    f0 += f1;
    f1 += f3;
    f2 += f0;
    f3 += f2;
    f0 += f2;
    f1 += f0;
    f2 += f3;
    f3 += f0;
    f0 += f2;
    f1 += f3;
    f2 += f0;
    f3 += f1;
  }

  test_values[0] = f0;
  test_values[1] = f1;
  test_values[2] = f2;
  test_values[3] = f3;
}

template <typename T>
[[gnu::noinline, gnu::optimize ("fast-math")]]
void IRAM_ATTR test_multiplication (void)
{
  T f0 = test_values[0];
  T f1 = test_values[1];
  T f2 = test_values[2];
  T f3 = test_values[3];

  for (int j = 0; j < N/16; j++)
  {
    f0 *= f3;
    f1 *= f2;
    f2 *= f1;
    f3 *= f0;
    f0 *= f1;
    f1 *= f3;
    f2 *= f0;
    f3 *= f2;
    f0 *= f2;
    f1 *= f1;
    f2 *= f3;
    f3 *= f0;
    f0 *= f0;
    f1 *= f1;
    f2 *= f2;
    f3 *= f3;
  }

  test_values[0] = f0;
  test_values[1] = f1;
  test_values[2] = f2;
  test_values[3] = f3;
}

template <typename T>
[[gnu::noinline, gnu::optimize ("fast-math")]]
void IRAM_ATTR test_multiply_accumulate (void)
{
  T f0 = test_values[0];
  T f1 = test_values[1];
  T f2 = test_values[2];
  T f3 = test_values[3];

  for (int j = 0; j < N/32; j++)
  {
    f0 += f3*f1;
    f1 += f2*f2;
    f2 += f1*f3;
    f3 += f0*f0;
    f0 += f1*f2;
    f1 += f3*f3;
    f2 += f0*f0;
    f3 += f2*f1;
    f0 += f3*f1;
    f1 += f2*f2;
    f2 += f1*f3;
    f3 += f0*f0;
    f0 += f1*f2;
    f1 += f3*f3;
    f2 += f0*f0;
    f3 += f2*f1;
    /*
    f0 += f2*f3;
    f1 += f1*f0;
    f2 += f3*f1;
    f3 += f0*f2;
    f0 += f0*f3;
    f1 += f1*f1;
    f2 += f2*f2;
    f3 += f3*f3;
    */
  }

  test_values[0] = f0;
  test_values[1] = f1;
  test_values[2] = f2;
  test_values[3] = f3;
}


void run_test (const char* name, std::function<void (void)> func)
{
  std::printf ("%s ... \n", name);

  for (unsigned int i = 0; i < M; ++i)
  {
    auto start_time = std::chrono::high_resolution_clock::now ();

    if (func)
      func ();
    else
      __builtin_unreachable ();

    auto end_time = std::chrono::high_resolution_clock::now ();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds> (end_time - start_time);

    std::printf ("f0 = %lf f1 = %lf f2 = %lf f3 = %lf\n",
		 (double)test_values[0], (double)test_values[1],
		 (double)test_values[2], (double)test_values[3]);

    std::printf ("%lf ns / insn\n", duration.count () / (double)N);
    std::printf ("%lf MOPS\n\n", (1000000000.0 / (duration.count () / (double)N)) / 1000000.0);
  }
}



extern "C"
void app_main (void)
{
  test_values[0] = 1;
  test_values[1] = 2;
  test_values[2] = 3;
  test_values[3] = 4;

  run_test ("int addition", &test_addition<int>);
  run_test ("int multiplication", &test_multiplication<int>);
  run_test ("int multiply-accumulate", &test_multiply_accumulate<int>);
  run_test ("float addition", &test_addition<float>);
  run_test ("float multiplication", &test_multiplication<float>);
  run_test ("float multiply-accumulate", &test_multiply_accumulate<float>);
}