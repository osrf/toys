#include <stdint.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#define N 6000

struct sysinfo mem_info;

int64_t get_physical_memory()
{
  sysinfo(&mem_info);
  int64_t total_mem = mem_info.totalram;
  total_mem *= mem_info.mem_unit;
  return total_mem;
}

void get_cpu_usage(int64_t &user, int64_t &userlow, int64_t &sys, int64_t &idle)
{
  FILE* fp;

  fp = fopen("/proc/stat", "r");
  fscanf(fp, "cpu %Ld %Ld %Ld %Ld", &user, &userlow, &sys, &idle);
  fclose(fp);
}

double calculate_cpu_usage(int64_t &luser, int64_t &luserlow, int64_t &lsys, int64_t &lidle, double &lpercent)
{
  int64_t user, userlow, sys, idle;
  get_cpu_usage(user, userlow, sys, idle);
  if (user < luser || userlow < luserlow || sys < lsys || idle < lidle) {
    return -1.0;
  }
  double percent;
  int64_t total;
  total = (user - luser) + (userlow - luserlow) + (sys - lsys);
  percent = total;
  total += (idle - lidle);
  if (total == 0) {
    return lpercent;
  }
  percent /= total;
  percent *= 100.0;
  luser = user;
  luserlow = userlow;
  lsys = sys;
  lidle = idle;
  lpercent = percent;
  return percent;
}

int
main()
{
  double t0 = 0.0;
  double ti;
  double* t = new double[N];
  double* cpu = new double[N];
  int64_t* mem = new int64_t[N];
  struct timeval tv;
  struct timespec ts;
  double dt;
  int64_t luser, luserlow, lsys, lidle;
  double lpercent = 0.0;
  get_cpu_usage(luser, luserlow, lsys, lidle);

  ts.tv_sec = 0;
  ts.tv_nsec = 1000000;

  int i = 0;
  while(i < N)
  {
    gettimeofday(&tv, NULL);
    ti = tv.tv_sec*1e3 + tv.tv_usec/1e3;
    dt = ti - t0;
    t[i] = dt;
    t0 = ti;
    if ((i % 10) == 0) {
      cpu[i] = calculate_cpu_usage(luser, luserlow, lsys, lidle, lpercent);
      mem[i] = get_physical_memory();
    } else {
      cpu[i] = cpu[i-1];
      mem[i] = mem[i-1];
    }
    // fprintf(stderr, "%li %li %li %li\n", luser, luserlow, lsys, lidle);
    i++;
    nanosleep(&ts, NULL);
  }
  
  for(int i=0;i<N;i++)
  {
    printf("%f, %f, %li\n", t[i], cpu[i], mem[i]);
  }
  
  delete[] t, cpu, mem;
  return 0;
}
