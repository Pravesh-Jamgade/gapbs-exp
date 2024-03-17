#include <iostream>

#include "magic.h"


void call()
{
    SimRoiStart();
    printf("b");
    SimRoiEnd();
}

int main()
{
    // SimRoiStart();
    // int* arr;
    // arr = (int*) calloc(10000, sizeof(int));
    // for(int i=0; i< 10000; i++)
    //     arr[i] = i+1;
    // SimRoiEnd();

    SimRoiStart();
    printf("a");
    call();
    SimRoiEnd();




    return  0;
}