#include "heuristic.hpp"
#include <algorithm>
#include <cmath>
#define DIMENSION 3
#define LAMBDA 0.2

const heuristic_func heuristic_funcs[] = {default_compare,surface_area,curve_complexity};

int default_compare(neuron net, int depth){
    return depth%DIMENSION;
}

int surface_area(neuron net, int depth){
    return true;
}

// DOI: 1011/cgf.142647
// Curve Complexity Heuristic KD-trees for Neighborhood-based Exploration of 3D Curves
int curve_complexity(neuron net, int depth){
    double C[DIMENSION];
    for(int i = 0; i < DIMENSION; i++){
        C[i] = 0.0; // C(T) = C_transversal(T) + C_backtrack(T) + nT_dist

        auto half = net.begin() + (net.size()/2);
        std::nth_element(net.begin(), half, net.end(), [i](compartment* lhs, compartment* rhs){
            return lhs->get(i) < rhs->get(i);
        });

        // C_transversal(T) = T_traverse + T_dist(P(T_l|T)*l + P(T_r|T)*r)
        // P(T_l|T) = Vol(T_l) / Vol(T) || P(T_r|T) = 1 - P(T_l|T)
        // Vol(T) = The AABB volume of T.
        // T_dist = 1 && T_traverse = 0.2

        double min_l[DIMENSION], max_l[DIMENSION], min_t[DIMENSION], max_t[DIMENSION];
        for(int j = 0; j < DIMENSION; j++){
            min_l[j] = min_t[j] = std::numeric_limits<double>::max();
            max_l[j] = max_t[j] = std::numeric_limits<double>::min();
        }

        // Find the min and max of the left and top half of the neuron.
        for(auto it = net.begin(); it != half; it++){
            for(int j = 0; j < DIMENSION; j++){
                min_t[j] = min_l[j] = std::min(min_l[j], (*it)->get(j));
                max_t[j] = max_l[j] = std::max(max_l[j], (*it)->get(j));
            }
        }
        for(auto it = half; it != net.end(); it++){
            for(int j = 0; j < DIMENSION; j++){
                min_t[j] = std::min(min_t[j], (*it)->get(j));
                max_t[j] = std::max(max_t[j], (*it)->get(j));
            }
        }

        double V_l = 1.0, V_t = 1.0;
        for(int j = 0; j < DIMENSION; j++){
            V_l *= max_l[j] - min_l[j];
            V_t *= max_t[j] - min_t[j];
        }

        // C_transversal(T) = T_traverse + T_dist(P(T_l|T)*l + P(T_r|T)*r)
        C[i] += 0.2 + (half- net.begin()) * (V_l/V_t) + (net.end() - half - 1) * (1 - V_l/V_t);

        // C_backtrack(T) = lambda * ( log_{n/l}(n) + log_{n/r}(n) )/2
        C[i] += 0.5 * LAMBDA * (std::log(net.size())/std::log(net.size()/(half-net.begin())) + std::log(net.size())/std::log(net.size()/(net.end()-half-1)));

        // nT_dist = n * T_dist
        C[i] += net.size() * 1.0;

    }

    return std::min_element(C, C+DIMENSION) - C;
}

int variance_heuristic(neuron net, int depth){
    if(net.size() < 2) return depth%DIMENSION;

    int count = 0;
    double M[DIMENSION], S[DIMENSION];
    for(int i=0;i<DIMENSION;i++){
        M[i]=0.0;
        S[i]=0.0;
    }

    /*  Based on Doald E. Knuth. The art of computer programming. Vol. 2. Page: 232  */
    double delta;
    for(compartment* k : net){
        count++;
        for(int i=0;i<DIMENSION;i++){
            delta = k->get(i) - M[i];
            M[i] += delta / count;
            S[i] += delta * (k->get(i) - M[i]);
        }
    }

    return std::max_element(S,S+DIMENSION)-S;
}