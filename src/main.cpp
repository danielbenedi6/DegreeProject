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
#include "heuristic.hpp"

void usage(char name[]){
	std::cout << "Usage:" << name << " [param] filename" << std::endl;
	std::cout << "Param: (Default -c)" << std::endl;
	std::cout << "    -s          Serial Kd-tree. Default" << std::endl;
	std::cout << "    -p0         Parallel Kd-tree without heuristics." << std::endl;
	std::cout << "    -p1         Parallel Kd-tree with Surface Area heuristic." << std::endl;
	std::cout << "    -p2         Parallel Kd-tree with Curve Complexity heuristic." << std::endl;
	std::cout << "    -p3         Parallel Kd-tree with ToBeDefined heuristic." << std::endl;
    std::cout << "swc_file    File with the neuron description with SWC format." << std::endl;
    std::cout << "rpl_file    File with the neuronal network replication. Each line " << std::endl;
    std::cout << "            represents one neuron and it has multiple triplets. If" << std::endl;
    std::cout << "            the first element is a 0 means that the operation is" << std::endl;
    std::cout << "            translation, if it is a 1 it will be rotation. The next" << std::endl;
    std::cout << "            element says which axis (0->X, 1->Y, 2->Z). The third" << std::endl;
    std::cout << "            element stablish the amount of rotation (radians) or the" << std::endl;
    std::cout << "            amount of translation (micrometers)." << std::endl;
}

void print(node* root, int depth){
    if(root != nullptr){
        for(int i = 0; i < depth-1; i++)
            std::cout << "|\t";
        if (depth > 0)
            std::cout << "|--";

        std::cout << "(" << root->data->x << "," << root->data->y << "," << root->data->z  << ") -> " << root->data->sample << ":" << root->data->type << " = " << root->data->radius << std::endl;
        print(root->left, depth+1);
        print(root->right, depth+1);
    }
}

int main(int argc, char* argv[]){
	std::string swc_file, rpl_file;
	bool parallel = false;
	int heuristic_id = 0;

	switch(argc){
		case 3:
            swc_file = argv[1];
            rpl_file = argv[2];
			break;
		case 4:
            swc_file = argv[2];
            rpl_file = argv[3];

			if(argv[1][0] != '-'){
				std::cout << "Unkown parameter: \"" << argv[1] << "\"" << std::endl;
				usage(argv[0]);
				return 128;
			}
			if(argv[1][1] == 'p'){
				parallel = true;
                heuristic_id = argv[1][2] - '0';
				if(heuristic_id < 0 || heuristic_id > 3){
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


    //std::cout << "Reading file: " << swc_file;
    auto neuron_description = parseSWC(swc_file);
    auto network = parseRPL(rpl_file, neuron_description);
    //std::cout << " Done!" << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<node*> nodes;
    if(parallel){
        for(const neuron& n : network){
            node* tree = build_parallel(n, 0, nullptr, heuristic_funcs[heuristic_id]);
            nodes.push_back(tree);
        }
        end = std::chrono::high_resolution_clock::now();
    }else{
        for(const neuron& n : network){
            node* tree = build_serial(neuron_description, 0, nullptr);
            nodes.push_back(tree);
        }
        end = std::chrono::high_resolution_clock::now();
    }

    //print(tree, 0);
    for(node* tree : nodes)
        free(tree);

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << duration << " us" << std::endl;

    return 0;
}

