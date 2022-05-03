#include "heuristic.hpp"
#include <algorithm>
#include <cmath>
#include <tuple>
#include <limits>

#define DIMENSION 3
#define LAMBDA 0.2

const heuristic_func heuristic_funcs[] = {default_compare,surface_area,curve_complexity, variance_heuristic, variance_ratio_heuristic};

// Default heuristic function - Spatial Median splitting
partition default_compare(neuron& net, int depth){
    const int dimension = depth% DIMENSION;
    auto m = net.begin()+net.size()/2;
    std::nth_element(net.begin(), m, net.end(), [dimension](compartment* lhs, compartment* rhs){
        return lhs->get(dimension) < rhs->get(dimension) || (lhs->get(dimension) == rhs->get(dimension) && lhs->sample < rhs->sample);
    });
    m = net.begin()+net.size()/2;
    neuron lhs(net.begin(), m), rhs(m+1, net.end());

    net.clear();
    auto result = new std::tuple<int, compartment*, neuron, neuron>(dimension, *m,lhs, rhs);

    return result;
}

// DOI: 10.1109/RT.2006.280216
// On building fast kd-Trees for Ray Tracing, and on doing that in O(N log N)
partition surface_area(neuron& net, int depth){
    double Chat = std::numeric_limits<double>::max();

    auto best_partition =  new std::tuple<int, compartment*, neuron, neuron>(0, nullptr, neuron(), neuron());
    for(int k = 0; k < DIMENSION; k++){
        std::vector<std::tuple<compartment*,double,int>> E = std::vector<std::tuple<compartment*,double,int>>();
        for(compartment* t : net){
            E.emplace_back(t,t->get(k), 2);
            E.emplace_back(t,t->get(k), 1);
        }
        std::sort(E.begin(),E.end(),[](const std::tuple<compartment*,double,int>& lhs, const std::tuple<compartment*,double,int>& rhs){
            return std::get<1>(lhs) < std::get<1>(rhs) || ( std::get<1>(lhs) == std::get<1>(rhs) && std::get<2>(lhs) < std::get<2>(rhs));
        });

        int Nl = 0, Np = 0, Nr = net.size();
        for(long unsigned int i = 0; i < E.size(); i++){
            auto p = E[i];
            int pPlus = 0, pMinus = 0, pEqual = 0;
            while(i < E.size() && std::abs(std::get<1>(E[i]) - std::get<1>(p)) < 1e-3 && std::get<2>(E[i]) == 0){
                pMinus++; i++;
            }
            while(i < E.size() && std::abs(std::get<1>(E[i]) - std::get<1>(p)) < 1e-3 && std::get<2>(E[i]) == 1){
                pEqual++; i++;
            }
            while(i < E.size() &&  std::abs(std::get<1>(E[i]) - std::get<1>(p)) < 1e-3 && std::get<2>(E[i]) == 2){
                pPlus++; i++;
            }
            Np = pEqual; Nr -= pEqual; Nl -= pEqual;

            neuron left, right, middle;
            for(compartment * c : net){
                if(c == std::get<0>(p)) continue;
                if((c->get(k) - std::get<1>(p)) < -1e-3){
                    left.push_back(c);
                }else if((c->get(k) - std::get<1>(p)) > 1e-3){
                    right.push_back(c);
                }else{
                    middle.push_back(c);
                }
            }

            /*
             * C(P_L,P_R,N_L,N_R) = lambda(p)(K_T+K_I*(P_L*N_L+P_R*N_R))
             */

            double lambda = left.size() == 0 || right.size() == 0 ? 0.8 : 1.0;
            // CpL = C(P_L,P_R, N_L+N_p, N_R)
            double CpL = lambda * (0.2 + 1.0*(double(left.size())/double(net.size())*double(Nl+Np) + double(right.size())/double(net.size())*double(Nr)));
            double CpR = lambda * (0.2 + 1.0*(double(left.size())/double(net.size())*double(Nl) + double(right.size())/double(net.size())*double(Nr+Np)));

            if(CpL < CpR && CpL < Chat){
                Chat = CpL;
                left.insert(left.end(), middle.begin(), middle.end());
                std::get<0>(*best_partition) = k;
                std::get<1>(*best_partition) = std::get<0>(p);
                std::get<2>(*best_partition) = left;
                std::get<3>(*best_partition) = right;
            }else if(CpR < Chat){
                Chat = CpR;
                right.insert(right.end(), middle.begin(), middle.end());
                std::get<0>(*best_partition) = k;
                std::get<1>(*best_partition) = std::get<0>(p);
                std::get<2>(*best_partition) = left;
                std::get<3>(*best_partition) = right;
            }
            Nl += pPlus; Nr += pMinus; Np = 0;
        }
    }

    net.clear();
    return best_partition;
}

// DOI: 10.1111/cgf.142647
// Curve Complexity Heuristic KD-trees for Neighborhood-based Exploration of 3D Curves
partition curve_complexity(neuron& net, int depth){
    if(net.size() <= 2) return default_compare(net, depth);

    double C = std::numeric_limits<double>::max();
    int dim = -1;
    for(int i = 0; i < DIMENSION; i++){
        double C_i = 0.0; // C(T) = C_transversal(T) + C_backtrack(T) + nT_dist

        auto half = net.begin() + (net.size()/2);
        std::nth_element(net.begin(), half, net.end(), [i](compartment* lhs, compartment* rhs){
            return lhs->get(i) < rhs->get(i);
        });
        half = net.begin() + (net.size()/2);
        

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
        C_i += 0.2 + (half- net.begin()) * (V_l/V_t) + (net.end() - half - 1) * (V_r/V_t);

        // C_backtrack(T) = lambda * ( log_{n/l}(n) + log_{n/r}(n) )/2
        C_i += 0.5 * LAMBDA * (std::log(net.size())/std::log(net.size()/(half-net.begin())) + std::log(net.size())/std::log(net.size()/(net.end()-half-1)));

        // nT_dist = n * T_dist
        C_i += net.size() * 1.0;

        if(C_i < C){
            C = C_i;
            dim = i;
        }
    }
    auto m = net.begin() + (net.size()/2);
    std::nth_element(net.begin(), m, net.end(), [dim](compartment* lhs, compartment* rhs){
        return lhs->get(dim) < rhs->get(dim);
    });
    m = net.begin() + (net.size()/2);
    auto result = new std::tuple<int, compartment*, neuron, neuron>(dim, *m, neuron(net.begin(), m), neuron(m+1, net.end()));

    net.clear();
    return result;
}

partition variance_heuristic(neuron& net, int depth){
    if(net.size() < 2) return default_compare(net, depth);

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

    int dim = std::max_element(S, S+DIMENSION) - S;

    return default_compare(net, dim);
}

/*
 * function variance(Cloud) returns best Phat
 * S_best = infinity
 * Phat = {}
 * for k = 1..3
 *   S_k = VarianceInDimension(Cloud, k)
 *   SortByDimension(Cloud, k)
 *   S_l = 0.0 : 1..|Cloud|; M_l = 0.0 : 1..|Cloud|
 *   S_r = 0.0 : |Cloud|;..1 M_r = 0.0 : |Cloud|..1
 *   for all p_i in Cloud {i = 1..|Cloud|}
 *      delta = p_i_k - M_l[i-1];
 *      M_l[i] = M_l[i-1] + delta / i;
 *      S_l[i] = S_l[i-1] + delta * (p_i_k - M_l[i]);
 *   endfor
 *   for all p_i in Cloud {i = 1..|Cloud|}
 *      delta = p_i_k - M_r[i+1];
 *      M_r[i] = M_r[i+1] + delta / i;
 *      S_r[i] = S_r[i+1] + delta * (p_i_k - M_r[i]);
 *      if (S_l[i] + S_r[i])/S_k < S_best then
 *          S_best = (S_l[i] + S_r[i])/S_k;
 *          Phat = p_i;
 *      fi
 *   endfor
 * endfor
 * return Phat
 * function \in O(k*(N*log(N) + 2*N)) = O(k*
 */
partition variance_ratio_heuristic(neuron& net, int depth){
    if(net.size() <= 2) return default_compare(net, depth);

    double S_best = std::numeric_limits<double>::max();
    auto result = new std::tuple<int, compartment*, neuron, neuron>(-1, nullptr, neuron(), neuron());

    for(int k = 0; k < DIMENSION; k++){
        //Sort by Dimension k
        std::sort(net.begin(), net.end(), [k](const compartment* a, const compartment* b){
            return a->get(k) < b->get(k);
        });

        std::vector<double> S_l(net.size(), 0.0), S_r(net.size(), 0.0);
        std::vector<double> M_l(net.size(), 0.0), M_r(net.size(), 0.0);

        //Compute S_l and M_l
        for(long unsigned int i = 0; i < net.size(); i++){
            const compartment* p = net[i];
            double delta = p->get(k) - M_l[i];
            M_l[i] += delta / (i+1);
            S_l[i] += delta * (p->get(k) - M_l[i]);

            const compartment* q = net[net.size()-(i+1)];
            delta = q->get(k) - M_r[net.size()-(i+1)];
            M_r[net.size()-(i+1)] = q->get(k)/(i+1);
            S_r[net.size()-(i+1)] = delta * (q->get(k) - M_r[net.size()-(i+1)]);
        }

        //Compute S_r and M_r
        for(long unsigned int i = 0; i < net.size(); i++){
            if((S_l[i] + S_r[i]) < S_best){
                S_best = (S_l[i] + S_r[i]);
                std::get<0>(*result) = k;
                std::get<1>(*result) = net[i];
                std::get<2>(*result) = neuron(net.begin(), net.begin()+i);
                std::get<3>(*result) = neuron(net.begin()+i+1, net.end());
            }
        }
    }

    if(std::get<0>(*result) == -1) return default_compare(net, depth);

    net.clear();
    return result;
}

/*
 * function variance(Cloud) returns best Phat
 * S_best = infinity
 * Phat = {}
 * for k = 1..3
 *   SortByDimension(Cloud, k)
 *   S_k = VarianceInDimension(Cloud, k)
 *   for all p in Cloud
 *      S_l = 0.0; M_l = 0.0;
 *      S_r = 0.0; M_r = 0.0;
 *      count = 0;
 *      for all p' in Cloud\{p}
 *          count = count + 1;
 *          if p_k > p'k then
 *              delta = p'_k - M_l;
 *              M_l = M_l + delta / count;
 *              S_l = S_l + delta * (p'_k - M_l);
 *          else
 *              delta = p'_k - M_r;
 *              M_r = M_r + delta / count;
 *              S_r = S_r + delta * (p'_k - M_r);
 *          fi
 *      endfor
 *      if (S_l + S_r)/S_k < S_best then
 *          S_best = (S_l + S_r)/S_k;
 *          Phat = p;
 *      fi
 *   endfor
 * endfor
 * return Phat
 *
 *
 * function \in O(kN*N*log(N))
 */

/*
 * function variance(Cloud) returns best Phat
 * S_best = infinity
 * Phat = {}
 * for k = 1..3
 *   S_k = VarianceInDimension(Cloud, k)
 *   SortByDimension(Cloud, k)
 *   S_l = 0.0 : 1..|Cloud|; M_l = 0.0 : 1..|Cloud|
 *   S_r = 0.0 : |Cloud|;..1 M_r = 0.0 : |Cloud|..1
 *   for all p_i in Cloud {i = 1..|Cloud|}
 *      delta = p_i_k - M_l[i-1];
 *      M_l[i] = M_l[i-1] + delta / i;
 *      S_l[i] = S_l[i-1] + delta * (p_i_k - M_l[i]);
 *   endfor
 *   for all p_i in Cloud {i = 1..|Cloud|}
 *      delta = p_i_k - M_r[i+1];
 *      M_r[i] = M_r[i+1] + delta / i;
 *      S_r[i] = S_r[i+1] + delta * (p_i_k - M_r[i]);
 *      if (S_l[i] + S_r[i])/S_k < S_best then
 *          S_best = (S_l[i] + S_r[i])/S_k;
 *          Phat = p_i;
 *      fi
 *   endfor
 * endfor
 * return Phat
 * function \in O(k*(N*log(N) + 2*N)) = O(k*
 */