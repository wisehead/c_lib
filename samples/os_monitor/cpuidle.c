/*******************************************************************************
 *      file name: cpuidle.c                                               
 *         author: Hui Chen. (c) 2019                             
 *           mail: chenhui13@baidu.com                                        
 *   created time: 2019/12/27-10:05:50                              
 *  modified time: 2019/12/27-10:05:50                              
 *******************************************************************************/
#include <stdio.h>

/** CPU state struct */
typedef struct cpu_packed {
    char name[20];
    unsigned int user;
    unsigned int nice;
    unsigned int system;
    unsigned int idle;
    unsigned int iowait;
    unsigned int irq;
    unsigned int softirq;
} cpu_stat_t;

/******************************************************************//**
Get CPU occupy from /proc/stat. */                                                                                                                                                                                                                              
void get_cpu_stat(cpu_stat_t* cpu_stat) {
    FILE *fd = NULL;
    char buff[256];
    fd = fopen ("/proc/stat", "r");
    fgets(buff, sizeof(buff), fd); 
 
    sscanf(buff, "%s %u %u %u %u %u %u %u",
            cpu_stat->name, &cpu_stat->user, &cpu_stat->nice,
            &cpu_stat->system, &cpu_stat->idle, &cpu_stat->iowait,
            &cpu_stat->irq, &cpu_stat->softirq);
 
    fclose(fd);
}

/******************************************************************//**
Calculate CPU idle. */
double cal_cpuidle(cpu_stat_t* one, cpu_stat_t* two) {                                                                                                                                                                                                          
    double one_total = 0.0; 
    double two_total = 0.0; 
    double one_idle  = 0.0; 
    double two_idle  = 0.0; 
    double cpu_idle  = 0.0; 
           
    one_total = (double) (one->user + one->nice + one->system + one->idle
                        + one->softirq + one->iowait + one->irq);
    two_total = (double) (two->user + two->nice + two->system + two->idle
                        + two->softirq + two->iowait + two->irq);
           
    one_idle = (double) (one->idle);
    two_idle = (double) (two->idle);
           
    if ((two_total - one_total) != 0) { 
        cpu_idle = (two_idle - one_idle)/(two_total - one_total) * 100.00;
    } else {
        cpu_idle = 100.00;
    }    
    return cpu_idle;
} 

int main()
{
	double cpu_idle;
	cpu_stat_t  cpu_stat_one; 
	cpu_stat_t  cpu_stat_two;
	get_cpu_stat(&cpu_stat_one);
	cpu_idle = cal_cpuidle(&cpu_stat_one, &cpu_stat_two);
	printf("cpuidle = [%lf]\n", cpu_idle);
}
