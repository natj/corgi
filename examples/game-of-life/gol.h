#pragma once

#include <vector>

#include "../../cell.h"
#include "../../corgi.h"
#include "../../toolbox/dataContainer.h"



namespace gol {


/// Snapshot patch of a CA simulation
class Mesh {

  public:

  /// patch dimensions
  int Nx;
  int Ny;

  int halo = 1;

  /// Internal 2D mesh storing the values
  std::vector<int> mesh;

  /// ctor
  Mesh(int Nx, int Ny);

  // Indexing with +1 halo regions around the array
  int indx(const int i, const int j) const {
    return (i+halo) + (Nx +2*halo)*(j+halo);
  }

  /// 2D access operator for values
  int operator () (const int i, const int j) const {
    return mesh[ indx(i,j) ];
  }

  int &operator () (const int i, const int j) {
    return mesh[ indx(i,j) ];
  }


  void copy_vert(Mesh& rhs, int lhsI, int rhsI);

  void copy_horz(Mesh& rhs, int lhsJ, int rhsJ);


};


/// Simulation grid
class Grid : public corgi::Node {

  public:
    Grid(size_t nx, size_t ny) : corgi::Node(nx, ny) { }

    ~Grid() { };

    // std::string pet_shop();

    /// Cycle data containers of each cell forward
    // void cycle() {
    //   for (auto& it: cells) {
    //     auto cellptr = std::dynamic_pointer_cast<CA_tile>( it.second );
    //     cellptr->data.cycle();
    //   }
    // }

};



/// Small local cellular automata patch
class CA_tile : public corgi::Cell {

  public:

    typedef CA_tile Tile_t;
    typedef std::shared_ptr<CA_tile> Tileptr;

    CA_tile(size_t i, size_t j, 
             int o, 
             size_t nx, size_t ny
             ) : corgi::Cell(i, j, o, nx, ny) { }

    ~CA_tile() { };

    // extending the base class
    datarotators::DataContainer<Mesh> data;

    void add_data(Mesh m);

    Mesh& get_data();

    Mesh* get_dataptr();

    Mesh& get_new_data();

    void update_boundaries(Grid& grid);


    /// step forward
    void cycle() { data.cycle(); }

};



class Solver {


  public:
    void solve(CA_tile& cell);



};





} // end of namespace gol
