#include "heuristic.hpp"
#include <algorithm>
#include <cmath>
#define DIMENSION 3
#define LAMBDA 0.2

const heuristic_func heuristic_funcs[] = {default_compare,surface_area,curve_complexity, variance_heuristic};

// Default heuristic function - Spatial Median splitting
int default_compare(neuron net, int depth){
    return depth%DIMENSION;
}

// DOI: 10.1109/RT.2006.280216
// On building fast kd-Trees for Ray Tracing, and on doing that in O(N log N)
int surface_area(neuron net, int depth){
    double C[DIMENSION];
    for(int  i = 0; i < DIMENSION; i++){
        C[i] = 0.0;

        auto half = net.begin() + (net.size()/2);
        std::nth_element(net.begin(), half, net.end(), [i](compartment* lhs, compartment* rhs){
            return lhs->get(i) < rhs->get(i);
        });

        double  min_l[DIMENSION], max_l[DIMENSION],
                min_r[DIMENSION], max_r[DIMENSION],
                min_t[DIMENSION], max_t[DIMENSION];
        for(int j = 0; j < DIMENSION; j++){
            min_l[j] = min_r[j] = min_t[j] = std::numeric_limits<double>::max();
            max_l[j] = max_r[j] = max_t[j] = std::numeric_limits<double>::min();
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
                min_r[j] = std::min(min_r[j], (*it)->get(j));
                max_r[j] = std::max(max_r[j], (*it)->get(j));
            }
        }

        double A_l = 0.0, A_r = 0.0, A_t = 0.0;
        for(int j = 0; j < DIMENSION; j++){
            A_l += 2*(max_l[j] - min_l[j]);
            A_r += 2*(max_r[j] - min_r[j]);
            A_t += 2*(max_t[j] - min_t[j]);
        }

        // C[V] = Kt + Ki * (SA(V_l)/SA(V) * |T_L| + SA(V_r)/SA(V) * |T_R|)
        C[i] = 0.2 + 1.0*(double(half-net.begin())*A_l/A_t + double(net.end()-half-1)*A_r/A_t);
    }

    return std::min_element(C, C+DIMENSION) - C;
}

// DOI: 10.1111/cgf.142647
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

        double  min_l[DIMENSION], max_l[DIMENSION],
                min_r[DIMENSION], max_r[DIMENSION],
                min_t[DIMENSION], max_t[DIMENSION];
        for(int j = 0; j < DIMENSION; j++){
            min_l[j] = min_r[j] = min_t[j] = std::numeric_limits<double>::max();
            max_l[j] = max_r[j] = max_t[j] = std::numeric_limits<double>::min();
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
                min_r[j] = std::min(min_r[j], (*it)->get(j));
                max_r[j] = std::max(max_r[j], (*it)->get(j));
            }
        }

        double V_l = 1.0, V_r = 1.0, V_t = 1.0;
        for(int j = 0; j < DIMENSION; j++){
            V_l *= max_l[j] - min_l[j];
            V_r *= max_r[j] - min_r[j];
            V_t *= max_t[j] - min_t[j];
        }

        // C_transversal(T) = T_traverse + T_dist(P(T_l|T)*l + P(T_r|T)*r)
        C[i] += 0.2 + (half- net.begin()) * (V_l/V_t) + (net.end() - half - 1) * (V_r/V_t);

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