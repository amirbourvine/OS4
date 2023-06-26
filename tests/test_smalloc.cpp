//
// Created by amirb on 26/06/2023.
//

#include "header_2.h"

int main(){
    void* b1 = smalloc(10);
    print();
    int* b2 = (int*)smalloc(20);
    b2[0] = 2;
    print();
    void* b3 = smalloc(30);
    print();
    sfree(b2);
    print();
    int* b4 = (int*)srealloc(b2, 30);
    print();

    if(b2!=b4){
        std::cout << "SUCCESS" << std::endl;
    }
    else{
        std::cout << "FAIL" << std::endl;
    }

    if(b4[0]==2){
        std::cout << "SUCCESS" << std::endl;
    }
    else{
        std::cout << "FAIL" << std::endl;
    }

}

