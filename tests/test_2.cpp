//
// Created by amirb on 26/06/2023.
//
#include <iostream>
#include "header_2.h"

int main() {
    //smalloc success tests
    int num_elements = 5;
    int* array1 = (int*) smalloc(num_elements * sizeof(int));

    if (array1 == NULL) {
        std::cerr << "Memory allocation failed!\n";
        return 1;
    }

    for (int i = 0; i < num_elements; i++) {
        array1[i] = 1;
    }

    for (int i = 0; i < num_elements; i++) {
        if(array1[i] != 1){
            std::cerr << "Memory allocation failed!\n";
            return 1;
        }
    }

    if(_num_free_blocks() != 0){
        std::cerr << "Incorrect number of free blocks\n";
        return 1;

    }

    if(_num_free_bytes() != 0){
        std::cerr << "Incorrect number of free bytes\n";
        return 1;

    }

    if(_num_allocated_blocks() != 1){
        std::cerr << "Incorrect number of allocated blocks\n";
        return 1;

    }

    if(_num_allocated_bytes() != sizeof(int) * num_elements){
        std::cerr << "Incorrect number of allocated bytes\n";
        return 1;

    }
    
    if(_num_meta_data_bytes() != _size_meta_data()){
        std::cerr << "Incorrect number of meta data bytes\n";
        return 1;

    }

    std::cout << "allocation 1 succeed\n";

    int* array2 = (int*) smalloc(num_elements * sizeof(int));

    if (array2 == NULL) {
        std::cerr << "Memory allocation failed!\n";
        return 1;

    }

    for (int i = 0; i < num_elements; i++) {
        array2[i] = 1;
    }

    for (int i = 0; i < num_elements; i++) {
        if(array2[i] != 1){
            std::cerr << "Memory allocation failed!\n";
            return 1;

        }
    }

    if(_num_free_blocks() != 0){
        std::cerr << "Incorrect number of free blocks\n";
        return 1;

    }

    if(_num_free_bytes() != 0){
        std::cerr << "Incorrect number of free bytes\n";
        return 1;

    }

    if(_num_allocated_blocks() != 2){
        std::cerr << "Incorrect number of allocated blocks\n";
        return 1;

    }

    if(_num_allocated_bytes() != sizeof(int) * num_elements * 2){
        std::cerr << "Incorrect number of allocated bytes\n";
        return 1;

    }

    if(_num_meta_data_bytes() != 2 * _size_meta_data()){
        std::cerr << "Incorrect number of meta data bytes\n";
        return 1;

    }

    std::cout << "allocation 2 succeed\n";

    int* array3 = (int*) smalloc(num_elements * sizeof(int));

    if (array3 == NULL) {
        std::cerr << "Memory allocation failed!\n";
        return 1;

    }

    for (int i = 0; i < num_elements; i++) {
        array3[i] = 1;
    }

    for (int i = 0; i < num_elements; i++) {
        if(array3[i] != 1){
            std::cerr << "Memory allocation failed!\n";
            return 1;

        }
    }

    if(_num_free_blocks() != 0){
        std::cerr << "Incorrect number of free blocks\n";
        return 1;

    }

    if(_num_free_bytes() != 0){
        std::cerr << "Incorrect number of free bytes\n";
        return 1;

    }

    if(_num_allocated_blocks() != 3){
        std::cerr << "Incorrect number of allocated blocks\n";
        return 1;

    }

    if(_num_allocated_bytes() != sizeof(int) * num_elements * 3){
        std::cerr << "Incorrect number of allocated bytes\n";
        return 1;

    }

    if(_num_meta_data_bytes() != _size_meta_data() * 3){
        std::cerr << "Incorrect number of meta data bytes\n";
        return 1;

    }
    
    std::cout << "allocation 3 succeed\n";

    //Free tests
    sfree(array1);

    return 0;
}

