#include <bits/stdc++.h>

#include "sim_api.h"
using namespace std;

/*

*/
#define SZ 10000000

int main()
{
    int *a = (int*)malloc(sizeof(int) * SZ);

    for(int i=0; i< SZ; i++)
    {
        a[i] = rand()%9999999;
    }

    uintptr_t start = reinterpret_cast<uintptr_t>(&a[0]);
    uintptr_t end = reinterpret_cast<uintptr_t>(&a[9999999]);

    SimUser(1,start);
    SimUser(2,end);

    SimRoiStart();
    int next_index = 0;
    for(int i=0; i< 10000; i++)
    {
        next_index = a[next_index];
        // cout << next_index << "," << &a[next_index] << '\n';
    }
    SimRoiEnd();

    return 0;

}