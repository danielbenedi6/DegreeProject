//*******************************************************
// File: main.cpp
// Author: Daniel Benedí García - 20000921-T111
// Date: 2022-03-24
// Coms: Main file to parse arguments and execute queries
//*******************************************************

#include <iostream>
#include <string>
#include <chrono>
#include <omp.h>
#include "SWC.hpp"
#include "K-d_tree.hpp"
void usage(char name[]){
	std::cout << "Usage:" << name << " [param] filename" << std::endl;
	std::cout << "Param: (Default -c)" << std::endl;
	std::cout << "    -s          Serial Kd-tree. Default" << std::endl;
	std::cout << "    -p0         Parallel Kd-tree without heuristics." << std::endl;
	std::cout << "    -p1         Parallel Kd-tree with Surface Area heuristic." << std::endl;
	std::cout << "    -p2         Parallel Kd-tree with Curve Complexity heuristic." << std::endl;
	std::cout << "    -p3         Parallel Kd-tree with ToBeDefined heuristic." << std::endl;
	std::cout << "filename    File with the neuronal network with SWC format." << std::endl;
}

void print(node* root, int depth){
    if(root != nullptr){
        for(int i = 0; i < depth-1; i++)
            std::cout << "|\t";
        if (depth > 0)
            std::cout << "|--";

        std::cout << "(" << root->data.x << "," << root->data.y << "," << root->data.z  << ") -> " << root->data.sample << ":" << root->data.type << " = " << root->data.radius << std::endl;
        print(root->left, depth+1);
        print(root->right, depth+1);
    }
}

int main(int argc, char* argv[]){
	std::string filename;
	bool parallel = false;
	int heuristic = 0;

	switch(argc){
		case 2:
			filename = argv[1];
			break;
		case 3:
			filename = argv[2];
			
			if(argv[1][0] != '-'){
				std::cout << "Unkown parameter: \"" << argv[1] << "\"" << std::endl;
				usage(argv[0]);
				return 128;
			}
			if(argv[1][1] == 'p'){
				parallel = true;
				heuristic = argv[1][2] - '0';
				if(heuristic < 0 || heuristic > 3){
					std::cout << "Unkown parameter: \"" << argv[1] << "\"" << std::endl;
					usage(argv[0]);
					return 128;
				}
			}else if(argv[1][1] != 's'){
				std::cout << "Unkown parameter: \"" << argv[1] << "\"" << std::endl;
				usage(argv[0]);
				return 128;
			}
			break;
		default:
			std::cout << "Incorrect number of arguments" << std::endl;
			usage(argv[0]);
			return 128;
	}


    //std::cout << "Reading file: " << filename;
    auto neurons = parseFile(filename);
    //std::cout << " Done!" << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    auto start = std::chrono::high_resolution_clock::now();
    node* tree;
    if(parallel){
        tree = build_parallel(neurons, neurons.begin(), neurons.end(), 0, nullptr);
        end = std::chrono::high_resolution_clock::now();
    }else{
        tree = build_serial(neurons, neurons.begin(), neurons.end(), 0, nullptr);
        end = std::chrono::high_resolution_clock::now();
    }

    //print(tree, 0);
    free(tree);

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << duration << " us" << std::endl;

    return 0;
}

