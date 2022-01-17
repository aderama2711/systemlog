#include "sys/types.h"
#include "sys/sysinfo.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
using std::cerr;
using std::endl;
#include <fstream>
using std::ofstream;
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <unistd.h>

struct sysinfo memInfo;

void init(){
    FILE* file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %llu %llu %llu %llu", &lastTotalUser, &lastTotalUserLow,
        &lastTotalSys, &lastTotalIdle);
    fclose(file);
}

double getCurrentValue(){
    double percent;
    FILE* file;
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;

    file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow,
        &totalSys, &totalIdle);
    fclose(file);

    if (totalUser < lastTotalUser || totalUserLow < lastTotalUserLow ||
        totalSys < lastTotalSys || totalIdle < lastTotalIdle){
        //Overflow detection. Just skip this value.
        percent = -1.0;
    }
    else{
        total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) +
            (totalSys - lastTotalSys);
        percent = total;
        total += (totalIdle - lastTotalIdle);
        percent /= total;
        percent *= 100;
    }

    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;

    return percent;
}


int main(){
	ofstream outdata;
	outdata.open("system.csv");
	if( !outdata){
		cerr << "Error FILE" << endl;
	}
	outdata << "Time,CPU used, RAM used, VRAM used" << endl;
	while (true){
		//VirtualMem
		sysinfo (&memInfo);
		long long totalVirtualMem = memInfo.totalram;
		//Add other values in next statement to avoid int overflow on right hand side...
		totalVirtualMem += memInfo.totalswap;
		totalVirtualMem *= memInfo.mem_unit;
		
		long long virtualMemUsed = memInfo.totalram - memInfo.freeram;
		//Add other values in next statement to avoid int overflow on right hand side...
		virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
		virtualMemUsed *= memInfo.mem_unit;
		
		long long totalPhysMem = memInfo.totalram;
		//Multiply in next statement to avoid int overflow on right hand side...
		totalPhysMem *= memInfo.mem_unit;
		
		long long physMemUsed = memInfo.totalram - memInfo.freeram;
		//Multiply in next statement to avoid int overflow on right hand side...
		physMemUsed *= memInfo.mem_unit;
		
		static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;
		
		double cpuused = getCurrentValue();
	
		
	    auto now = std::chrono::system_clock::now();
	    outdata << std::to_string(std::ctime(&now)) << "," << std::to_string(cpuused) "," << std::to_string(physMemUsed) << "," << std::to_string(virtualMemUsed) << endl;
	    
	    sleep(1);
	}
}





