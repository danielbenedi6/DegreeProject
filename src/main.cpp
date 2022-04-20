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
	std::cout << "Usage:" << name << " [param] swc_file rpl_file neuron compartment" << std::endl;
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
    std::cout << "            element establish the amount of rotation (radians) or the" << std::endl;
    std::cout << "            amount of translation (micrometers)." << std::endl;
    std::cout << "neuron      The number of the neuron in which the query will be executed" << std::endl;
    std::cout << "compartment The id of the compartment in which the query will be executed" << std::endl;
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
	int heuristic_id = 0, n, c;

	switch(argc){
		case 5:
            swc_file = argv[1];
            rpl_file = argv[2];
            n = std::stoi(argv[3]);
            c = std::stoi(argv[4]);
			break;
		case 6:
            swc_file = argv[2];
            rpl_file = argv[3];
            n = std::stoi(argv[4]);
            c = std::stoi(argv[5]);

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

    std::cout << neuron_description.size() << " compartments" << std::endl;
    std::cout << network.size() << " neurons" << std::endl;

    auto end_build = std::chrono::high_resolution_clock::now();
    auto start_build = std::chrono::high_resolution_clock::now();
    auto end_query = std::chrono::high_resolution_clock::now();
    auto start_query = std::chrono::high_resolution_clock::now();
    std::vector<node*> nodes;
    compartment* query = network[n][c];
    std::vector<compartment*> result;
    if(parallel){
        //=========================================================
        //===================== BUILD =============================
        //=========================================================
        for(neuron & i : network){
            node* tree = build_parallel(i, 0, nullptr, heuristic_funcs[heuristic_id]);
            nodes.push_back(tree);
        }
        end_build = std::chrono::high_resolution_clock::now();


        //=========================================================
        //===================== QUERY =============================
        //=========================================================
        start_query = std::chrono::high_resolution_clock::now();
        #pragma omp parallel for default(none) shared(nodes, query, result)
        for(node* & elem : nodes){
            compartment* res = find_nearest(elem, query, 3.0);
            if(res != nullptr){
                #pragma omp critical
                result.push_back(res);
            }
        }
        end_query = std::chrono::high_resolution_clock::now();
    }else{
        //=========================================================
        //===================== BUILD =============================
        //=========================================================
        for(neuron & i : network){
            node* tree = build_serial(i, 0, nullptr);
            nodes.push_back(tree);
        }
        end_build = std::chrono::high_resolution_clock::now();


        //=========================================================
        //===================== QUERY =============================
        //=========================================================
        start_query = std::chrono::high_resolution_clock::now();
        for(node* & elem : nodes){
            compartment* res = find_nearest(elem, query, 3.0);
            if(res != nullptr)
                result.push_back(res);
        }
        end_query = std::chrono::high_resolution_clock::now();
    }

    std::cout << result.size() << " hits" << std::endl;

    //print(tree, 0);
    for(node* tree : nodes)
        free(tree);

    auto duration_build = std::chrono::duration_cast<std::chrono::microseconds>(end_build - start_build).count();
    auto duration_query = std::chrono::duration_cast<std::chrono::microseconds>(end_query - start_query).count();
    std::cout << duration_build << " us" << std::endl;
    std::cout << duration_query << " us" << std::endl;

    return 0;
}

