//
// Created by amirb on 26/06/2023.
//
#include "header_2.h"
int main(){
    print();
    int* b1 = (int*)smalloc((128 * 1024));
    print();
    std::cout << "HERE1-MAIN" << std::endl;
    //b1[0] = 1;
    std::cout << "HERE2-MAIN" << std::endl;
    sfree(b1);
    std::cout << "HERE3-MAIN" << std::endl;
    print();
}
