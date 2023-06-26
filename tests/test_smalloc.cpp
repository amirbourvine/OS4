//
// Created by amirb on 26/06/2023.
//

#include "header_2.h"

int main(){
    int* b1 = (int*)scalloc(5, sizeof(int));
    print();
    for(int i = 0; i<5; i++){
        if(b1[i]!=0){
            std::cout << "FAIL" << std::endl;
        }
    }
    int* b2 = (int*)smalloc(5*sizeof(int));
    b2[0] = 4;
    print();
    sfree(b2);
    int* b3 = (int*)scalloc(5, sizeof(int));
    print();
    for(int i = 0; i<5; i++){
        if(b1[i]!=0){
            std::cout << "FAIL" << std::endl;
        }
    }
}

