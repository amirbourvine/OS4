//
// Created by Elad on 26/06/2023.
//

#include <iostream>
#include "header_1.h"

int main() {
    int num_elements;
    int* array;

    std::cout << "Enter the number of elements: \n";
    std::cin >> num_elements;

    // Dynamically allocate memory for the array
    array = (int*)smalloc(num_elements * sizeof(int));

    if (array == NULL) {
        std::cerr << "Memory allocation failed!\n";
        return 0;
    }

    for (int i = 0; i < num_elements; i++) {
        array[i] = 1;
    }

    std::cout << "You entered the following integers:\n";
    for (int i = 0; i < num_elements; i++) {
        std::cout << array[i] << " ";
    }
    std::cout << "\n";

    return 0;
}
