#include <iostream>
#include <iomanip>
#include <limits>
#include <bits/stdc++.h>
#include "sim_api.h"

class Me{

    class A{
        int** copied;
    public:
        A(int** a):copied(a)
        {
        }

        typedef int** iterator;
        iterator begin(){
            return &copied[0];
        }

        iterator end(){
            return &copied[10];
        }
    };

public:
    Me(int *b)
    {
        a = (int**)malloc(sizeof(int*)*100);
        for(int i=0; i<100; i++)
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

    int b[100000]={0};
    for(int i=0; i< 100000; i++)
    {
        b[i]=i*100+1;
    }

    uintptr_t start = reinterpret_cast<uintptr_t>(&b[0]);
    uintptr_t end = reinterpret_cast<uintptr_t>(&b[99999]);

    SimUser(3, start);
    SimUser(4, end);
    SimUser(765, 0);

    std::cout << "******************************\n";
    std::cout << "           [   Application ]               \n";
    std::cout << std::hex << "start=" << start << '\n';
    std::cout << std::hex << "end=" << end << '\n';
    std::cout << "******************************\n";

    Me m(b);

    SimRoiStart();
    for(int i=0; i< 10; i++)
    {
        std::cout << std::hex << &b[i] << '\n';
    }

    for(int* a: m.start())
    {
        std::cout << std::dec << *a <<","<< std::hex << a << '\n';
    }
    // for(int i=0; i< 10; i++)//int& a: m.start()
    // {
    //     int t = *(b+i);
    //     *(b+i) = 1;
    //     // std::cout << (b+i) << ", new=" << *(b+i) <<  '\n';//", old=" << t <<
    // }
    SimRoiEnd();

    // int a[]={1,2,3,4,5};// neigh
    // int **p = (int**)malloc(sizeof(int**)*5);// index
    // for(int i=0; i< 5; i++)
    // {
    //     p[i] = (int*)malloc(sizeof(int*));
    // }

    // for(int i=0; i< 5; i++)
    // {
    //     p[i]=&a[i];
    // }

    // for(int i=0; i< 5; i++)
    // {
    //     std::cout << &p[i] << ',' << p[i] << "=" << &a[i] << ',' << *p[i] << '\n';
    // }

    // // new
    // int **q;
    // q = new int*[5];
    // for(int i=0; i< 5; i++)
    // {
    //     q[i]=&a[i];
    // }

    // for(int i=0; i< 5; i++)
    // {
    //     std::cout << &q[i] << ',' << q[i] << "=" << &a[i] << ',' << *q[i] << '\n';
    // }

    // std::cout << '\n';

    // std::cout << q << "," << q+4 << '\n';

    // if(1>1)
    //     std::cout << "Ok";
    return 0;
}