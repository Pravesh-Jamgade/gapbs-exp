// Copyright (c) 2018, The Hebrew University of Jerusalem (HUJI, A. Barak)
// See LICENSE.txt for license details

#include <algorithm>
#include <cinttypes>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "benchmark.h"
#include "bitmap.h"
#include "builder.h"
#include "command_line.h"
#include "graph.h"
#include "pvector.h"
#include "timer.h"


/*
GAP Benchmark Suite
Kernel: Connected Components (CC)
Authors: Michael Sutton, Scott Beamer

Will return comp array labelling each vertex with a connected component ID

This CC implementation makes use of the Afforest subgraph sampling algorithm [1],
which restructures and extends the Shiloach-Vishkin algorithm [2].

[1] Michael Sutton, Tal Ben-Nun, and Amnon Barak. "Optimizing Parallel 
    Graph Connectivity Computation via Subgraph Sampling" Symposium on 
    Parallel and Distributed Processing, IPDPS 2018.

[2] Yossi Shiloach and Uzi Vishkin. "An o(logn) parallel connectivity algorithm"
    Journal of Algorithms, 3(1):57–67, 1982.
*/


using namespace std;


// Place nodes u and v in same component of lower component ID
void Link(NodeID u, NodeID v, pvector<NodeID>& comp) {
  NodeID p1 = comp[u];
  NodeID p2 = comp[v];
  while (p1 != p2) {
    NodeID high = p1 > p2 ? p1 : p2;
    NodeID low = p1 + (p2 - high);
    NodeID p_high = comp[high];
    // Was already 'low' or succeeded in writing 'low'
    if ((p_high == low) ||
        (p_high == high && compare_and_swap(comp[high], high, low)))
      break;
    p1 = comp[comp[high]];
    p2 = comp[low];
  }
}


// Reduce depth of tree for each component to 1 by crawling up parents
void Compress(const Graph &g, pvector<NodeID>& comp) {
  #pragma omp parallel for schedule(dynamic, 16384)
  for (NodeID n = 0; n < g.num_nodes(); n++) {
    while (comp[n] != comp[comp[n]]) {
      comp[n] = comp[comp[n]];
    }
  }
}


NodeID SampleFrequentElement(const pvector<NodeID>& comp,
                             int64_t num_samples = 1024) {
  std::unordered_map<NodeID, int> sample_counts(32);
  using kvp_type = std::unordered_map<NodeID, int>::value_type;
  // Sample elements from 'comp'
  std::mt19937 gen;
  std::uniform_int_distribution<NodeID> distribution(0, comp.size() - 1);
  for (NodeID i = 0; i < num_samples; i++) {
    NodeID n = distribution(gen);
    sample_counts[comp[n]]++;
  }
  // Find most frequent element in samples (estimate of most frequent overall)
  auto most_frequent = std::max_element(
    sample_counts.begin(), sample_counts.end(),
    [](const kvp_type& a, const kvp_type& b) { return a.second < b.second; });
  float frac_of_graph = static_cast<float>(most_frequent->second) / num_samples;
  std::cout
    << "Skipping largest intermediate component (ID: " << most_frequent->first
    << ", approx. " << static_cast<int>(frac_of_graph * 100)
    << "% of the graph)" << std::endl;
  return most_frequent->first;
}


pvector<NodeID> Afforest(const Graph &g, int32_t neighbor_rounds = 2) {
  pvector<NodeID> comp(g.num_nodes());

  // Initialize each node to a single-node self-pointing tree
  #pragma omp parallel for
  for (NodeID n = 0; n < g.num_nodes(); n++)
    comp[n] = n;

  // Process a sparse sampled subgraph first for approximating components.
  // Sample by processing a fixed number of neighbors for each node (see paper)
  for (int r = 0; r < neighbor_rounds; ++r) {
  #pragma omp parallel for schedule(dynamic,16384)
    for (NodeID u = 0; u < g.num_nodes(); u++) {
      for (NodeID v : g.out_neigh(u, r)) {
        // Link at most one time if neighbor available at offset r
        Link(u, v, comp);
        break;
      }
    }
    Compress(g, comp);
  }

  // Sample 'comp' to find the most frequent element -- due to prior
  // compression, this value represents the largest intermediate component
  NodeID c = SampleFrequentElement(comp);

  // Final 'link' phase over remaining edges (excluding largest component)
  if (!g.directed()) {
    #pragma omp parallel for schedule(dynamic, 16384)
    for (NodeID u = 0; u < g.num_nodes(); u++) {
      // Skip processing nodes in the largest component
      if (comp[u] == c)
        continue;
      // Skip over part of neighborhood (determined by neighbor_rounds)
      for (NodeID v : g.out_neigh(u, neighbor_rounds)) {
        Link(u, v, comp);
      }
    }
  } else {
    #pragma omp parallel for schedule(dynamic, 16384)
    for (NodeID u = 0; u < g.num_nodes(); u++) {
      if (comp[u] == c)
        continue;
      for (NodeID v : g.out_neigh(u, neighbor_rounds)) {
        Link(u, v, comp);
      }
      // To support directed graphs, process reverse graph completely
      for (NodeID v : g.in_neigh(u)) {
        Link(u, v, comp);
      }
    }
  }
  // Finally, 'compress' for final convergence
  Compress(g, comp);
  return comp;
}


void PrintCompStats(const Graph &g, const pvector<NodeID> &comp) {
  cout << endl;
  unordered_map<NodeID, NodeID> count;
  for (NodeID comp_i : comp)
    count[comp_i] += 1;
  int k = 5;
  vector<pair<NodeID, NodeID>> count_vector;
  count_vector.reserve(count.size());
  for (auto kvp : count)
    count_vector.push_back(kvp);
  vector<pair<NodeID, NodeID>> top_k = TopK(count_vector, k);
  k = min(k, static_cast<int>(top_k.size()));
  cout << k << " biggest clusters" << endl;
  for (auto kvp : top_k)
    cout << kvp.second << ":" << kvp.first << endl;
  cout << "There are " << count.size() << " components" << endl;
}


// Verifies CC result by performing a BFS from a vertex in each component
// - Asserts search does not reach a vertex with a different component label
// - If the graph is directed, it performs the search as if it was undirected
// - Asserts every vertex is visited (degree-0 vertex should have own label)
bool CCVerifier(const Graph &g, const pvector<NodeID> &comp) {
  unordered_map<NodeID, NodeID> label_to_source;
  for (NodeID n : g.vertices())
    label_to_source[comp[n]] = n;
  Bitmap visited(g.num_nodes());
  visited.reset();
  vector<NodeID> frontier;
  frontier.reserve(g.num_nodes());
  for (auto label_source_pair : label_to_source) {
    NodeID curr_label = label_source_pair.first;
    NodeID source = label_source_pair.second;
    frontier.clear();
    frontier.push_back(source);
    visited.set_bit(source);
    for (auto it = frontier.begin(); it != frontier.end(); it++) {
      NodeID u = *it;
      for (NodeID v : g.out_neigh(u)) {
        if (comp[v] != curr_label)
          return false;
        if (!visited.get_bit(v)) {
          visited.set_bit(v);
          frontier.push_back(v);
        }
      }
      if (g.directed()) {
        for (NodeID v : g.in_neigh(u)) {
          if (comp[v] != curr_label)
            return false;
          if (!visited.get_bit(v)) {
            visited.set_bit(v);
            frontier.push_back(v);
          }
        }
      }
    }
  }
  for (NodeID n=0; n < g.num_nodes(); n++)
    if (!visited.get_bit(n))
      return false;
  return true;
}


int main(int argc, char* argv[]) {
  CLApp cli(argc, argv, "connected-components-afforest");
  if (!cli.ParseArgs())
    return -1;
  Builder b(cli);
  Graph g = b.MakeGraph();
  auto CCBound = [](const Graph& gr){ return Afforest(gr); };


  //saurabh very start
  NodeID *pattern = nullptr;
  pattern = new NodeID[100];
  pattern[0] = 1;
  pattern[0+11] = 2;
  pattern[0+7] = 4;
  pattern[0+19] = 4;
  pattern[0+21] = 4;
  std::cout << " Pattern_Before_All:  "<< std::hex << &pattern[0] << " " << &pattern[11] << " " << &pattern[7] << " " << &pattern[19] << std::dec <<std::endl;

  //Pravesh to get Index and Neigh
  NodeID** index = g.get_index_array();
  NodeID* neigh = g.get_neighbor_array();
  
  std::cout << " Index: " << std::hex << &index[0] << "," << &index[g.num_nodes()-1] << '\n';
  std::cout << " Neigh: " << std::hex << &neigh[0] << "," << &neigh[g.get_edge_array_len()-1] << '\n';
  
  //saurabh Pattern for neigh start
  {
  NodeID *pattern2 = nullptr;
  pattern2 = new NodeID[100];

  std::cout << "start " << neigh[0] << " " << &neigh[0] << std::endl;
  
  pattern2 = &neigh[0];

  //saving the pointer
  pattern[0] = pattern2[0-19-1];
  pattern[0+11] = pattern2[0-8-1];
  pattern[0+7] = pattern2[0-12-1];
  pattern[0+19] = pattern2[0-1];
  
  //apply pattern
  pattern2[0-19-1] = 5;
  pattern2[0-8-1] = 4;
  pattern2[0-12-1] = 4;
  pattern2[0-1] = 4;
  
  //Restoring pointer
  pattern2[0-19-1] = pattern[0];
  pattern2[0-8-1] = pattern[0+11];
  pattern2[0-12-1] = pattern[0+7];
  pattern2[0-1] = pattern[0+19];
  }

  //saurabh Pattern for neigh end
  {
  NodeID *pattern3 = nullptr;
  pattern3 = new NodeID[100];

  pattern3 = &neigh[g.get_edge_array_len()-1];
  
  //saving the pointer
  pattern[0] = pattern3[-20];  //0
  pattern[11] = pattern3[-9];  //11
  pattern[7] = pattern3[-13];  //7
  pattern[19] = pattern3[-1];  //19

  //apply pattern
  pattern3[-20] = 5;
  pattern3[-9] = 4;
  pattern3[-13] = 4;
  pattern3[-1] = 4;

  //Restoring pointer
  pattern3[-20] = pattern[0];
  pattern3[-9] = pattern[11];
  pattern3[-13] = pattern[7];
  pattern3[-1] = pattern[19];
  }

  //saurabh Pattern for index start
  {
  NodeID ** elements = new NodeID* [100+ g.num_nodes()];
  NodeID ** elements2 = new NodeID* [100 + g.num_nodes()];
  elements2 = &index[0];

  //saving the pointer
  elements[0] = elements2[0];
  elements[0+11] = elements2[11];
  elements[0+7] = elements2[7];
  elements[0+19] = elements2[19];
   
  int i = 4;

  //apply pattern
  elements2[0] = &i; //&pattern[0];
  elements2[11] = &i; // &pattern[0];
  elements2[7] = &i; // &pattern[0];
  elements2[19] = &i; // &pattern[0];

  std::cout << " element2:  "<< std::hex << &elements2[0] << " " << &elements2[11] << " " << &elements2[7] << " " << &elements2[19] << std::dec <<std::endl;
  // std::cout << " element2:  "<< std::hex << &elements2[-20 + g.num_nodes()] << " " << &elements2[-9 + g.num_nodes()] << " " << &elements2[-13 + g.num_nodes()] << " " << &elements2[-1 + g.num_nodes()] << std::dec <<std::endl;
  
  //Restoring pointer
  elements2[0] = elements[0];
  elements2[11] = elements[0+11];
  elements2[7] = elements[0+7];
  elements2[19] = elements[0+19];
  
  
  //saurabh Pattern for index end
  NodeID ** elements1 = new NodeID* [100+ g.num_nodes()];
  //saving the pointer
  elements1[0] = elements2[0-19-1 + g.num_nodes()];
  elements1[0+11] = elements2[0-8-1 + g.num_nodes()];
  elements1[0+7] = elements2[0-12-1 + g.num_nodes()];
  elements1[0+19] = elements2[0-1 + g.num_nodes()];
   
  // int i = 4;

  //apply pattern
  elements2[0-19-1 + g.num_nodes()] = &i; //&pattern[0];
  elements2[0-8-1 + g.num_nodes()] = &i; // &pattern[0];
  elements2[0-12-1 + g.num_nodes()] = &i; // &pattern[0];
  elements2[0-1 + g.num_nodes()] = &i; // &pattern[0];

  // std::cout << " element2:  "<< std::hex << &elements2[-20] << " " << &elements2[-9] << " " << &elements2[-13] << " " << &elements2[-1] << std::dec <<std::endl;
  std::cout << " element2:  "<< std::hex << &elements2[-20 + g.num_nodes()] << " " << &elements2[-9 + g.num_nodes()] << " " << &elements2[-13 + g.num_nodes()] << " " << &elements2[-1 + g.num_nodes()] << std::dec <<std::endl;
  
  //Restoring pointer
  elements2[0-19-1 + g.num_nodes()] = elements1[0];
  elements2[0-8-1 + g.num_nodes()] = elements1[0+11];
  elements2[0-12-1 + g.num_nodes()] = elements1[0+7];
  elements2[0-1 + g.num_nodes()] = elements1[0+19];
  }

  // saurabh

  BenchmarkKernel(cli, g, CCBound, PrintCompStats, CCVerifier);
  return 0;
}
