#pragma once

#include <vector>

#include "common.h"


namespace corgi {

class Cell {

  public:
    // Order here is fixed for mpi_cell_t

    /// unique cell ID
    uint64_t cid;

    /// MPI rank of who owns me
    int owner;

    /// coarse mpiGrid grid indices
    size_t i, j;

    /// If I am a virtual cell, who do I share the values the most.
    int top_virtual_owner;

    /// how many times do I have to be sent to others
    size_t communications;

    /// How many virtual neighbors do I have
    size_t number_of_virtual_neighbors = 0;

    /// Cell type listing
    bool local;

    std::vector<int> types;

    /// Global grid dimensions (needed for wrapping boundaries)
    size_t Nx = 0;
    size_t Ny= 0;


    /// initalize cell according to its location (i,j) and owner (o)
    Cell(size_t i, size_t j, int o, size_t Nx, size_t Ny) {
      this->i     = i;
      this->j     = j;
      this->owner = o;

      this->Nx    = Nx;
      this->Ny    = Ny;
    }

    /*! \brief *virtual* base class destructor 
     * NOTE: this needs to be virtual so that child classes can be 
     * destroyed.
     */
    virtual ~Cell() { }

    /// return mpiGrid index
    const std::tuple<size_t, size_t> index() {
      return std::make_tuple( i, j );
    }

    /// default periodic x boundary condition
    virtual size_t xwrap(int i) {
        while (i < 0) {
            i += Nx;
        }
        while (i >= Nx) {
            i -= Nx;
        }
        return size_t(i);
    }


    /// default periodic y boundary condition
    virtual size_t ywrap( int j ) {
        while (j < 0) {
            j += Ny;
        }
        while (j >= Ny) {
            j -= Ny;
        }
        return size_t(j);
    }


    /// return index of cells in relative to my position
    const std::tuple<size_t, size_t> neighs(int ir, int jr) {
      size_t ii = xwrap( (int)this->i + ir );
      size_t jj = ywrap( (int)this->j + jr );
      return std::make_tuple( ii, jj );
    }


    /// Return full neighborhood around me
    std::vector< std::tuple<size_t, size_t> > nhood() {
      std::vector< std::tuple<size_t, size_t> > nh;
      for (int ir=-1; ir<=1; ir++) {
        for (int jr=-1; jr<=1; jr++) {
          if (!( ir == 0 && jr == 0 )) {
            nh.push_back( neighs(ir, jr) );
          }
        }
      }
      return nh;
    }


    /// Check if cell fulfills a single criteria
    bool is_type( int criteria ) {
      if( std::find(
            types.begin(), 
            types.end(), 
            criteria) 
          == types.end() 
        ) {
        return false;
      } 
      return true;
    }

    /// Vectorized version requiring cell to fulfill every criteria
    bool is_types( std::vector<int> criteria ) {
      for (auto crit: criteria) {
        if (is_type(crit))  {
          continue;
        } else {
          return false;
        }
      }

      // passed all criteria
      return true;
    }

}; // end of Cell class

}