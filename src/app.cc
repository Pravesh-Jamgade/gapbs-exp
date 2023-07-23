#include <bits/stdc++.h>
#include "sim_api.h"
#define SZ 100000
#define SY 99999
using namespace std;


int main()
{
    int *a = (int*) malloc(sizeof(int)*SZ);
    for(int i=0; i< SZ; i++)
    {
        a[i] = rand()%SY;
    }
    uintptr_t addr3s = reinterpret_cast<uintptr_t>(&a[0]);
    uintptr_t addr3e = reinterpret_cast<uintptr_t>(&a[SY]);
    SimUser(1, addr3s);
        SimUser(2, addr3e);
        SimUser(765, 0);

    int index = 0;

    #ifdef LD
        cout << "LD LD LD LD LD LD\n";
        cout << "LD LD LD LD LD LD\n";

        SimRoiStart();
        for(int i=0; i< SZ; i++)
        {
            index = *(a+index);
        }
        SimRoiEnd();
    #endif

    #ifdef NOP
        cout << "NOP NOP NOP NOP NOP NOP\n";
        cout << "NOP NOP NOP NOP NOP NOP\n";

        SimRoiStart();
        for(int i=0; i< SZ; i++)
        {
            index = *(a+index);
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
        }
        SimRoiEnd();
    #endif

    return 0;
}