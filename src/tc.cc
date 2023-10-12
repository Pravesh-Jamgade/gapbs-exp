// Copyright (c) 2015, The Regents of the University of California (Regents)
// See LICENSE.txt for license details

// Encourage use of gcc's parallel algorithms (for sort for relabeling)
#ifdef _OPENMP
  #define _GLIBCXX_PARALLEL
#endif

#include <algorithm>
#include <cinttypes>
#include <iostream>
#include <vector>

#include "benchmark.h"
#include "builder.h"
#include "command_line.h"
#include "graph.h"
#include "pvector.h"

#include <set>
#include <unistd.h> 

#include "magic.h"
/*
GAP Benchmark Suite
Kernel: Triangle Counting (TC)
Author: Scott Beamer

Will count the number of triangles (cliques of size 3)

Requires input graph:
  - to be undirected
  - no duplicate edges (or else will be counted as multiple triangles)
  - neighborhoods are sorted by vertex identifiers

Other than symmetrizing, the rest of the requirements are done by SquishCSR
during graph building.

This implementation reduces the search space by counting each triangle only
once. A naive implementation will count the same triangle six times because
each of the three vertices (u, v, w) will count it in both ways. To count
a triangle only once, this implementation only counts a triangle if u > v > w.
Once the remaining unexamined neighbors identifiers get too big, it can break
out of the loop, but this requires that the neighbors to be sorted.

Another optimization this implementation has is to relabel the vertices by
degree. This is beneficial if the average degree is high enough and if the
degree distribution is sufficiently non-uniform. To decide whether or not
to relabel the graph, we use the heuristic in WorthRelabelling.
*/


using namespace std;

size_t OrderedCount(const Graph &g) {
  size_t total = 0;

  NodeID addr3s = reinterpret_cast<uintptr_t>(&total);
  NodeID addr3e = reinterpret_cast<uintptr_t>(&total);
  SimUser(addr3s,addr3e,3);
  // SimUser(addr3e, 3);

  std::cout  << std::hex << "PROPERY: " << addr3s << "," << addr3e << '\n';

  #pragma omp parallel for reduction(+ : total) schedule(dynamic, 64)
  for (NodeID u=0; u < g.num_nodes(); u++) {
    for (NodeID v : g.out_neigh(u)) {
      if (v > u)
        break;
      auto it = g.out_neigh(u).begin();
      for (NodeID w : g.out_neigh(v)) {
        if (w > v)
          break;
        while (*it < w)
          it++;
        if (w == *it)
          total++;
      }
    }
  }
  return total;
}


// heuristic to see if sufficently dense power-law graph
bool WorthRelabelling(const Graph &g) {
  int64_t average_degree = g.num_edges() / g.num_nodes();
  if (average_degree < 10)
    return false;
  SourcePicker<Graph> sp(g);
  int64_t num_samples = min(int64_t(1000), g.num_nodes());
  int64_t sample_total = 0;
  pvector<int64_t> samples(num_samples);
  for (int64_t trial=0; trial < num_samples; trial++) {
    samples[trial] = g.out_degree(sp.PickNext());
    sample_total += samples[trial];
  }
  sort(samples.begin(), samples.end());
  double sample_average = static_cast<double>(sample_total) / num_samples;
  double sample_median = samples[num_samples/2];
  return sample_average / 1.3 > sample_median;
}


// uses heuristic to see if worth relabeling
size_t Hybrid(const Graph &g) {
  if (WorthRelabelling(g))
    return OrderedCount(Builder::RelabelByDegree(g));
  else
    return OrderedCount(g);
}


void PrintTriangleStats(const Graph &g, size_t total_triangles) {
  cout << total_triangles << " triangles" << endl;
}


// Compares with simple serial implementation that uses std::set_intersection
bool TCVerifier(const Graph &g, size_t test_total) {
  size_t total = 0;
  vector<NodeID> intersection;
  intersection.reserve(g.num_nodes());
  for (NodeID u : g.vertices()) {
    for (NodeID v : g.out_neigh(u)) {
      auto new_end = set_intersection(g.out_neigh(u).begin(),
                                      g.out_neigh(u).end(),
                                      g.out_neigh(v).begin(),
                                      g.out_neigh(v).end(),
                                      intersection.begin());
      intersection.resize(new_end - intersection.begin());
      total += intersection.size();
    }
  }
  total = total / 6;  // each triangle was counted 6 times
  if (total != test_total)
    cout << total << " != " << test_total << endl;
  return total == test_total;
}


int main(int argc, char* argv[]) {
  CLApp cli(argc, argv, "triangle count");
  if (!cli.ParseArgs())
    return -1;
  Builder b(cli);
  Graph g = b.MakeGraph();
  
  NodeID** index_arr_base = g.get_index_array();
  NodeID* edge_arr_base = *index_arr_base;

  uint64_t addr1s = reinterpret_cast<uint64_t>(&index_arr_base[0]);
  uint64_t addr1e = reinterpret_cast<uint64_t>(&index_arr_base[g.num_nodes()-1]);
  uint64_t addr2s = reinterpret_cast<uint64_t>(&edge_arr_base[0]);
  uint64_t addr2e = reinterpret_cast<uint64_t>(&edge_arr_base[g.num_edges()-1]);

  std::cout << std::hex << "INDEX: " << addr1s << "," << addr1e << '\n';
  std::cout << std::hex  << "EDGE: " << addr2s << "," << addr2e << '\n';

  SimUser(addr1s,addr1e,1);
  // SimUser(, 1);

  SimUser(addr2s,addr2e,2);
  // SimUser(addr2e, 2);

  if (g.directed()) {
    cout << "Input graph is directed but tc requires undirected" << endl;
    return -2;
  }

  // fstream f;
  // string name = "index.stat";
  // f.open(name.c_str(), std::fstream::app | std::fstream::in);

  // fstream e;
  // name = "edge.stat";
  // e.open(name.c_str(), std::fstream::app | std::fstream::in);

  // for(int i=0; i< g.num_nodes(); i++){
  //   f << &index_arr_base[i] << '\n';
  // }
  
  // for(int i=0; i< g.num_edges(); i++){
  //   e << &edge_arr_base[i] << '\n';
  // }

  // g.PrintStats();

  // cout << "-----------------------------------------------------------\n";
  // cout << "[APPLICATION] Running Kernel.............\n";
  // cout << "-----------------------------------------------------------\n";

  BenchmarkKernel(cli, g, Hybrid, PrintTriangleStats, TCVerifier);
  return 0;
}
