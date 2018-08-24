#pragma once


#include <array>
#include <vector>
#include <tuple>
#include <iostream>
#include <algorithm>

#include "common.h"
#include "internals.h"


namespace corgi {


// Data storage struct for communication members
struct Communication {

    /// MPI rank of who owns me
    int owner;

    /// If I am a virtual tile, who do I share the values the most.
    int top_virtual_owner;

    /// how many times do I have to be sent to others
    size_t communications;

    /// How many virtual neighbors do I have
    size_t number_of_virtual_neighbors = 0;

    /// tile type listing
    bool local;

    std::vector<int> types;
};



/*! \brief Tile oject 
 *
 * This is the smallest storage unit of the framework. Internally this should host 
 * a mesh/grid/particles/etc.
 */
template<std::size_t D>
class Tile
{

  public:

    /// unique tile ID
    uint64_t cid;


    // Order here is fixed for mpi_tile_t
    Communication communication;

    /// coarse mpiGrid grid indices
    corgi::internals::tuple_of<D, size_t> index;

    /// Global grid dimensions (needed for wrapping boundaries)
    std::array<size_t, D> lengths;

    /// tile boundaries
    std::array<double, D> mins;
    std::array<double, D> maxs;

      
    // using default ctor
    // TODO: are there side effects?
    //Tile() = default;

    /*! \brief *virtual* base class destructor 
     * NOTE: this needs to be virtual so that child classes can be 
     * destroyed.
     */
    virtual ~Tile() = default;

    /*

    /// default periodic x boundary condition
    corgi::internals::enable_if_t< (D>=1), size_t>
    xwrap(int iw) {
      auto Nx = static_cast<int>(lengths[0]);

      while (iw < 0) { iw += Nx; }
      while (iw >= Nx) { iw -= Nx; }
      return static_cast<size_t>(iw);
    }


    /// default periodic y boundary condition
    corgi::internals::enable_if_t< (D>=2), size_t>
    ywrap(int jw) {
      auto Ny = static_cast<int>(lengths[1]);

      while (jw < 0) { jw += Ny; }
      while (jw >= Ny) { jw -= Ny; }
      return static_cast<size_t>(jw);
    }

    /// default periodic z boundary condition
    corgi::internals::enable_if_t< (D>=3), size_t>
    zwrap(int kw) {
      auto Nz = static_cast<int>(lengths[2]);

      while (kw < 0) { kw += Nz; }
      while (kw >= Nz) { kw -= Nz; }
      return static_cast<size_t>(kw);
    }

    */

    /// general N-dim implementation of wrap
    size_t wrap(int ind, size_t d)
    {
      auto N = static_cast<int>(lengths[d]);
      while (ind < 0) { ind += N; }
      while (ind >= N) { ind -= N; }
      return static_cast<size_t>(ind);
    }


    /// return index of tiles in relative to my position
    template<typename... Indices>
    corgi::internals::enable_if_t< (sizeof...(Indices) == D) && 
    corgi::internals::are_integral<Indices...>::value, 
    const corgi::internals::tuple_of<D, size_t> > 
    neighs(Indices... indices_rel)
    {
        std::array<int, D>    rel = {{static_cast<int>(indices_rel)...}};
        //std::array<size_t, D> cur = {{index}};
        auto cur = corgi::internals::into_array(index);

        for(size_t i=0; i<D; i++) {
          cur[i] = static_cast<size_t>(
            wrap( static_cast<int>(rel[i]) + 
                  static_cast<int>(cur[i]), i)
          );
        }

        auto ret = corgi::internals::into_tuple(cur);
        return ret;
    }


    /// Return full neighborhood around me
    //std::vector< std::tuple<size_t, size_t> > nhood() {
    //  std::vector< std::tuple<size_t, size_t> > nh;
    //  for (int ir=-1; ir<=1; ir++) {
    //    for (int jr=-1; jr<=1; jr++) {
    //      if (!( ir == 0 && jr == 0 )) {
    //        nh.push_back( neighs(ir, jr) );
    //      }
    //    }
    //  }
    //  return nh;
    //}


    /// Check if tile fulfills a single criteria
    bool is_type( int criteria ) {
      if( std::find(
            communication.types.begin(), 
            communication.types.end(), 
            criteria) 
          == communication.types.end() 
        ) {
        return false;
      } 
      return true;
    }

    /// Vectorized version requiring tile to fulfill every criteria
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

    // --------------------------------------------------
    // (optional) tile geometry 

    /// set tile minimum limits
    void set_tile_mins(std::array<double, D> bbox)
    {
      mins = std::move(bbox);
    }

    /// set tile minimum limits
    void set_tile_maxs(std::array<double, D> bbox)
    {
      maxs = std::move(bbox);
    }


}; // end of Tile class

} // end of namespace corgi
