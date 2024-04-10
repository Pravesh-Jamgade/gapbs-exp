#include <iostream>

int main()
{

    asm volatile("nop");
    int* arr;
    arr = (int*) calloc(10000, sizeof(int));
    for(int i=0; i< 10000; i++)
        arr[i] = i+1;
    
    return  0;
}