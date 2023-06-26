//
// Created by amirb on 26/06/2023.
//

#include "header_2.h"

int main(){
    void* b1 = smalloc(10000000000);
    if(b1==NULL){
        std::cout << "SUCCESS" << std::endl;
    }
    else{
        std::cout << "FAIL" << std::endl;
    }
    void* b2 = smalloc(0);
    if(b2==NULL){
        std::cout << "SUCCESS" << std::endl;
    }
    else{
        std::cout << "FAIL" << std::endl;
    }


}

