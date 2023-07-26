#include <iostream>
#include <iomanip>
#include <limits>
#include <bits/stdc++.h>

#define SIZE 100000
#define END 99999 
#include "sim_api.h"

class Me{

    class A{
        int** copied;
    public:
        A(int** a):copied(a)
        {
        }

        typedef int* iterator;
        iterator begin(){
            return copied[0];
        }

        iterator end(){
            return copied[END];
        }
    };

public:
    Me(int *b)
    {
        a = (int**)malloc(sizeof(int*)*SIZE);
        for(int i=0; i<1e5; i++)
        {
            a[i]=(int*)malloc(sizeof(int));
            a[i]=&b[i];
        }
    }

    A start()
    {
        return A(a);
    }

    int**a;
};

int main() {

    int b[SIZE]={0};
    for(int i=0; i< SIZE; i++)
        b[i]=i+1;

    uintptr_t p = reinterpret_cast<uintptr_t>(&b[0]);
    uintptr_t q = reinterpret_cast<uintptr_t>(&b[END]);
    SimUser(3, p);
    SimUser(4, q);

    Me m(b);

    SimRoiStart();
    for(int a: m.start())
    {
        // std::cout << a << '\n';
    }
    SimRoiEnd();

    return 0;
}