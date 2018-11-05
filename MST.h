#ifndef STEINER_MST_H_DEFINED__
#define STEINER_MST_H_DEFINED__

#include "Net.h"
#include "Types.h"

#include <vector>

template<typename T>
class Graph {
public:
  struct EdgeType {
    size_t From, To;

    EdgeType() = default;
    EdgeType(size_t from, size_t to): From(from), To(to) {}
  };

private:
  std::vector<EdgeType> Edges;
  std::vector<T> Vertices;

public:
  Graph() = default;
  Graph(const Graph &) = delete;
  Graph(Graph &&) = default;
  void operator=(const Graph &) = delete;
  void operator=(Graph &&) = delete;
  ~Graph() = default;

  auto edges_begin() { return Edges.begin(); }
  auto edges_end() { return Edges.end(); }
  auto edges_begin() const { return Edges.begin(); }
  auto edges_end() const { return Edges.end(); }

  T &vertice(size_t Idx) { return Vertices[Idx]; }
  const T &vertice(size_t Idx) const { return Vertices[Idx]; }

  auto vertices_size() const { return Vertices.size(); }

  void swapVertices(std::vector<T> &Vs) {
    std::swap(Vertices, Vs);
  }

  void swapEdges(std::vector<EdgeType> &Es) {
    std::swap(Edges, Es);
  }

  void dump() const {
    std::cout << "Vertices:" << std::endl;
    for (size_t i = 0, e = Vertices.size(); i < e; ++i) {
      std::cout << i << " " << Vertices[i] << std::endl;
    }
    std::cout << "Edges:" << std::endl;
    for (auto E : Edges) {
      std::cout << E.From << " -> " << E.To << std::endl;
    }
  }

  // TODO: add methods for working with edges and vertices?
};

Unit getMSTLen(const Graph<Point> &G);

std::vector<typename Graph<Point>::EdgeType>
getMSTEdges(const Graph<Point> &G);
#endif
