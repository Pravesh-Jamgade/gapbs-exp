#include <bits/stdc++.h>
using namespace std;
#include <unistd.h>


#define M 1024
#define ML 1023
int main()
{
    int arr[M] = {0};
    uintptr_t x = reinterpret_cast<uintptr_t>(&arr[0]);
    uintptr_t y = reinterpret_cast<uintptr_t>(&arr[ML]);
    
    for(int i=0; i<M; i++)
    {
        arr[i]=i+32;
    }

    for(int i=0; i<M;)
    {
        // cout << "["<<i<<"]" << &arr[i] << '\n';
        i = arr[i];
    }

    for(int i=0; i<M; i++)
    {
        cout << "["<<i<<"]" << &arr[i] << '\n';
    }

    return 0;
}