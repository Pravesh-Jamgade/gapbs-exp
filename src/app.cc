<<<<<<< HEAD
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
    return 0;
}
// =======
// #include <bits/stdc++.h>
// #include "sim_api.h"
// #define SZ 100000
// #define SY 99999
// using namespace std;


// int main()
// {
//     int *a = (int*) malloc(sizeof(int)*SZ);
//     for(int i=0; i< SZ; i++)
//     {
//         a[i] = rand()%SY;
//     }
//     uintptr_t addr3s = reinterpret_cast<uintptr_t>(&a[0]);
//     uintptr_t addr3e = reinterpret_cast<uintptr_t>(&a[SY]);
//     SimUser(1, addr3s);
//         SimUser(2, addr3e);
//         SimUser(765, 0);

//     int index = 0;

//     #ifdef LD
//         cout << "LD LD LD LD LD LD\n";
//         cout << "LD LD LD LD LD LD\n";

//         SimRoiStart();
//         for(int i=0; i< SZ; i++)
//         {
//             index = *(a+index);
//         }
//         SimRoiEnd();
//     #endif

//     #ifdef NOP
//         cout << "NOP NOP NOP NOP NOP NOP\n";
//         cout << "NOP NOP NOP NOP NOP NOP\n";

//         SimRoiStart();
//         for(int i=0; i< SZ; i++)
//         {
//             index = *(a+index);
//             asm volatile("nop");
//             asm volatile("nop");
//             asm volatile("nop");
//             asm volatile("nop");
//             asm volatile("nop");
//         }
//         SimRoiEnd();
//     #endif

// >>>>>>> 23d1d3e0016b1fce17c038380bc2b2a3f4dde63d
//     return 0;
// }