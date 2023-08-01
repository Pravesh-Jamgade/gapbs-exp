#include <bits/stdc++.h>
using namespace std;
#include <unistd.h>

#include "sim_api.h"

#define M 1024
#define ML 1023
int main()
{
    int arr[M] = {0};
    uintptr_t x = reinterpret_cast<uintptr_t>(&arr[0]);
    SimUser(1, x);
    uintptr_t y = reinterpret_cast<uintptr_t>(&arr[ML]);
    SimUser(2, y);
    
    for(int i=0; i<M; i++)
    {
        arr[i]=i+32;
    }

    SimRoiStart();
    for(int i=0; i<M;)
    {
        // cout << "["<<i<<"]" << &arr[i] << '\n';
        i = arr[i];
    }
    SimRoiEnd();

    for(int i=0; i<M; i++)
    {
        cout << "["<<i<<"]" << &arr[i] << '\n';
    }

    return 0;
}