#pragma once

// #include <fmt/format.h>
// #include <fmt/format.cc>
// #include <fmt/string.h>
// #include <fmt/ostream.h>

#include <vector>
#include <algorithm>
#include <cmath>
#include <memory>
#include <unordered_map>
#include <cassert>
#include <initializer_list>
#include <sstream>
#include <utility>

#include "internals.h"
#include "toolbox/sparse_grid.h"
#include "tile.h"

//#include "mpi.h"
#include <mpi4cpp/mpi.h>
#include "communication.h"


namespace corgi {

namespace mpi = mpi4cpp::mpi;


/*! Individual node object that stores patches of grid in it.
 *
 * See:
 * - https://github.com/maddouri/hyper_array/
 * - https://github.com/astrobiology/orca_array
*/

template<std::size_t D>
class Node
{


  public:
      
  // --------------------------------------------------
  // definitions
  using size_type  = std::size_t;
  using index_type = std::size_t;
  using float_type = double;


  protected:

  // --------------------------------------------------
  /// number of elements in each dimension
  ::std::array<size_type, D> _lengths;

  /// start coordinates of each dimension
  ::std::array<float_type, D> _mins;
    
  /// ending coordinates of each dimension
  ::std::array<float_type, D> _maxs;

  /*! Global large scale block grid where information
   * of all the mpi processes are stored
   */
  corgi::tools::sparse_grid<int, D> _mpi_grid;

  // --------------------------------------------------
  private:
    
  // Mappings
  using TileID_t = uint64_t;
  using Tile_t   = corgi::Tile<D>;
  using Tileptr  = std::shared_ptr<Tile_t>;
  using Tile_map  = std::unordered_map<TileID_t, Tileptr>;



  public:

  /// Map with tile_id & tile data
  Tile_map tiles;


  public:
  // --------------------------------------------------
  // Python bindings for mpi_grid

  // get element
  template<typename... Indices>
  corgi::internals::enable_if_t< (sizeof...(Indices) == D) && 
  corgi::internals::are_integral<Indices...>::value, int > 
  py_get_mpi_grid(Indices... indices)  /*const*/
  {
    return _mpi_grid(indices...);
  }


  // set element
  template<typename... Indices>
  corgi::internals::enable_if_t< (sizeof...(Indices) == D) && 
  corgi::internals::are_integral<Indices...>::value, void > 
  py_set_mpi_grid(int val, Indices... indices) {
    _mpi_grid(indices...) = val;
  }



  public:

  // --------------------------------------------------
  // constructors

  /// mpi environment
  mpi::environment env;

  /// mpi communicator
  mpi::communicator comm;
    
  /// Uninitialized dimension lengths
  Node() :
    env(),
    comm()
  {};

  /// copy-constructor
  //Node(const Node& /*other*/) {};
  
  /// move constructor
  //Node(Node&& /*other*/) {}; // use std::move()
   
  /// set dimensions during construction time
  template<
    typename... DimensionLength,
    typename = corgi::internals::enable_if_t< (sizeof...(DimensionLength) == D) && 
               corgi::internals::are_integral<DimensionLength...>::value, void
    >
  > 
  Node(DimensionLength... dimension_lengths) :
    _lengths {{static_cast<size_type>(dimension_lengths)...}},
    _mpi_grid(dimension_lengths...),
    env(),
    comm()
  { }


  /*
   * try specializing handy shortcuts to symmetrize construction always assuming 3D input
  template< typename = corgi::internals::enable_if_t< (D == 1), void > > 
  Node(size_t i, size_t j, size_t k) :
    _lengths {{i}},
    _mpi_grid({{i}})
  { }

  template< typename = corgi::internals::enable_if_t< (D == 2), void > > 
  Node(size_t i, size_t j, size_t k) :
    _lengths {{i, j}},
    _mpi_grid({{i, j}})
  { }
  */
  

  /// Deallocate and free everything
  virtual ~Node() = default;

  public:
  
  // --------------------------------------------------
  // assignments
    
  /// copy assignment
  //Node& operator=(const Node& other)
  //{
  //  _lengths = other._lengths;

  //  return *this;
  //}


  /// move assignment
  //Node& operator=(const Node&& other)
  //{
  //  _lengths   = std::move(other._lengths);

  //  return *this;
  //}




  //public:
  // --------------------------------------------------
  // iterators
  /*
          iterator         begin()         noexcept { return iterator(data());                }
    const_iterator         begin()   const noexcept { return const_iterator(data());          }
          iterator         end()           noexcept { return iterator(data() + size());       }
    const_iterator         end()     const noexcept { return const_iterator(data() + size()); }
          reverse_iterator rbegin()        noexcept { return reverse_iterator(end());         }
    const_reverse_iterator rbegin()  const noexcept { return const_reverse_iterator(end());   }
          reverse_iterator rend()          noexcept { return reverse_iterator(begin());       }
    const_reverse_iterator rend()    const noexcept { return const_reverse_iterator(begin()); }
    const_iterator         cbegin()  const noexcept { return const_iterator(data());          }
    const_iterator         cend()    const noexcept { return const_iterator(data() + size()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end());   }
    const_reverse_iterator crend()   const noexcept { return const_reverse_iterator(begin()); }
  */



  public:
  // --------------------------------------------------
  // access grid configuration

  /// number of dimension
  static constexpr size_type dims() noexcept { return D; }

  /// length (run-time)
  size_type len(const size_type i) const
  {
    assert(i < D);
    return _lengths[i];
  }

  /// reference to the _lengths array
  const ::std::array<size_type, D>& lens() const noexcept
  {
    return _lengths;
  }
  
  /// starting location of i:th dimension (run-time)
  float_type min(const size_type i) const
  {
    assert(i < D);
    return _mins[i];
  }
    
  /// ending location of i:th dimension (run-time)
  float_type max(const size_type i) const
  {
    assert(i < D);
    return _maxs[i];
  }

  /// reference to the _mins array
  const ::std::array<float_type, D>& mins() const noexcept
  {
    return _mins;
  }
    
  /// reference to the _maxs array
  const ::std::array<float_type, D>& maxs() const noexcept
  {
    return _maxs;
  }


  // --------------------------------------------------
  // indexing
  private:
  
  template <typename... Indices>
  corgi::internals::enable_if_t< (sizeof...(Indices) == D) && 
  corgi::internals::are_integral<Indices...>::value,
        ::std::array<index_type, D>>
    _validate_index_range(Indices... indices) const
  {
    ::std::array<index_type, D> index_array = {{static_cast<index_type>(indices)...}};

    // check all indices and prepare an exhaustive report (in oss)
    // if some of them are out of bounds
    std::ostringstream oss;
    for (index_type i = 0; i < D; ++i)
    {
      if ((index_array[i] >= _lengths[i]) || (index_array[i] < 0))
      {
        oss << "Index #" << i << " [== " << index_array[i] << "]"
          << " is out of the [0, " << (_lengths[i]-1) << "] range. ";
      }
    }

    // if nothing has been written to oss then all indices are valid
    assert(oss.str().empty());
    return index_array;
  }

  /*! Computes the index coefficients assuming column-major order
   *
   *  what we compute:
   *        \f[
   *            \begin{cases}
   *            C_i = \prod_{j=i+1}^{n-1} L_j
   *            \\
   *            \begin{cases}
   *                i   &\in [0, \text{Dimensions - 1}] \\
   *                C_i &: \text{\_coeffs[i]}           \\
   *                L_j &: \text{\_lengths[j]}
   *            \end{cases}
   *            \end{cases}
   *        \f]
   *
   *  For row-major switch to:
   *  coeffs[i] = ct_accumulate(dimension_lengths, i + 1, Dimensions - i - 1,
   *                                        static_cast<size_type>(1),
   *                                        ct_prod<size_type>);
   *
   */
  std::array<size_type, D>
  compute_index_coeffs(const ::std::array<size_type, D>& dimension_lengths) const noexcept
  {
      std::array<size_type, D> coeffs;
      for (size_type i = 0; i < D; ++i)
      {
          coeffs[i] = corgi::internals::ct_accumulate(
              dimension_lengths,
              0,
              i,
              static_cast<size_type>(1),
              corgi::internals::ct_prod<size_type>);
      }
      return coeffs;
  }


  /// Actual Morton Z-ordering from index list
  // 
  // what we compute: coeff . indices
  //
  // i.e., inner product of accumulated coefficients vector and index vector
  constexpr index_type 
  _compute_index(
      const ::std::array<index_type, D>& index_array) const noexcept
  {
    return corgi::internals::ct_inner_product(
        compute_index_coeffs(_lengths), 0,
        index_array, 0, D,
        static_cast<index_type>(0),
        corgi::internals::ct_plus<index_type>,
        corgi::internals::ct_prod<index_type>);
  }

  public:
    
  /// tile IDs
  template<typename... Indices>
  corgi::internals::enable_if_t< (sizeof...(Indices) == D) && 
  corgi::internals::are_integral<Indices...>::value, index_type > 
  id(Indices... indices) const
  {
    return _compute_index( _validate_index_range(indices...) );
  }


  /// auxiliary function to unpack tuples
  template <size_t... Is>
  index_type id_impl(
      corgi::internals::tuple_of<D, size_t>& tuple, 
      std::index_sequence<Is...>)
  {
    return id( std::get<Is>(tuple)... );
  }

  /// unpack tuple into variadic argument list
  template<typename Indices = std::make_index_sequence<D>>
  index_type id( corgi::internals::tuple_of<D, size_t>& indices)
  {
      return id_impl(indices, Indices{} );
  }
  

  public:

  // --------------------------------------------------
  // apply SFINAE to create some shortcuts (when appropriate) 
  // NOTE: valid up to D=3 with x/y/z

  // return global grid sizes
  template<typename T = size_type>
  corgi::internals::enable_if_t< (D>=1), T> 
  get_Nx() { return _lengths[0]; }

  template<typename T = size_type>
  corgi::internals::enable_if_t< (D>=2), T> 
  get_Ny() { return _lengths[1]; }

  template<typename T = size_type>
  corgi::internals::enable_if_t< (D>=3), T> 
  get_Nz() { return _lengths[2]; }


  // return global grid limits
  template<typename T = size_type>
  corgi::internals::enable_if_t< (D>=1), T> 
  get_xmin() { return _mins[0]; }

  template<typename T = size_type>
  corgi::internals::enable_if_t< (D>=2), T> 
  get_ymin() { return _mins[1]; }

  template<typename T = size_type>
  corgi::internals::enable_if_t< (D>=3), T> 
  get_zmin() { return _mins[2]; }


  template<typename T = size_type>
  corgi::internals::enable_if_t< (D>=1), T> 
  get_xmax() { return _maxs[0]; }

  template<typename T = size_type>
  corgi::internals::enable_if_t< (D>=2), T> 
  get_ymax() { return _maxs[1]; }

  template<typename T = size_type>
  corgi::internals::enable_if_t< (D>=3), T> 
  get_zmax() { return _maxs[2]; }


  /// Set physical grid size
  void set_grid_lims(
      const ::std::array<float_type, D>& mins,
      const ::std::array<float_type, D>& maxs
      )
  {
    //_mins = std::move(mins);
    //_maxs = std::move(maxs);

    // explicitly avoid move semantics and copy
    // this is to make sure that python garbage collector
    // does not mess things up
    _mins = mins;
    _maxs = maxs;
  }



  public:
  // --------------------------------------------------
  // Tile addition etc. manipulation
    
  /// Add local tile to the node
  // void add_tile(Tile& tile) {
  void add_tile(
    Tileptr tileptr,
    corgi::internals::tuple_of<D, size_t> indices
    )
  {

    // claim unique ownership of the tile (for unique_ptr)
    // std::unique_ptr<corgi::Tile> tileptr = std::make_unique<corgi:Tile>(tile);
    // Tileptr tileptr = std::make_unique<Tile_t>(tile);
    
    // calculate unique global tile ID
    uint64_t cid = id( indices );

    // Erase any existing tiles to avoid emplace of doing nothing TODO: is this correct?
    tiles.erase(cid);

    tileptr->index               = indices;
    tileptr->cid                 = cid;
    tileptr->communication.cid   = cid;
    tileptr->communication.owner = comm.rank();
    tileptr->communication.local = true; //TODO Catch error if tile is not already mine?
    tileptr->lengths             = _lengths;


    // copy indices from tuple into D=3 array in Communication obj
    auto tmp = corgi::internals::into_array(indices);
    for(size_t i=0; i<D; i++) tileptr->communication.indices[i] = tmp[i];

    // tiles.emplace(cid, std::move(tileptr)); // unique_ptr needs to be moved
    tiles.emplace(cid, tileptr); // NOTE using c++14 emplace to avoid copying
    //tiles.insert( std::make_pair(cid, tileptr) ); // NOTE using c++14 emplace to avoid copying
    //tiles[cid] = tileptr;
    //tiles[cid] = tileptr;
      
    // add to my internal listing
    _mpi_grid( indices ) = comm.rank();
  }


  /// Shortcut for creating raw tiles with only the internal meta info.
  // to be used with message passing (w.r.t. add_tile that is for use with initialization)
  void create_tile(Communication& cm)
  {
    auto tileptr = std::make_shared<Tile_t>();
    tileptr->load_metainfo(cm);

    // additional node info
    tileptr->lengths = _lengths;
    // owner
    // local

    // add
    tiles.emplace(cm.cid, tileptr); // NOTE using c++14 emplace to avoid copying
    _mpi_grid( tileptr->index ) = cm.owner;
  }

  /// Update tile metadata
  void update_tile(Communication& cm)
  {
    auto& tile = get_tile(cm.cid);
    tile.load_metainfo(cm);
    _mpi_grid( tile.index ) = cm.owner;
  }


  /*! Return a vector of tile indices that fulfill a given criteria.  */
  std::vector<uint64_t> get_tile_ids(
      const bool sorted=false ) {
    std::vector<uint64_t> ret;

    for (auto& it: tiles) ret.push_back( it.first );

    // optional sort based on the tile id
    if (sorted && !ret.empty()) {
      std::sort(ret.begin(), ret.end());
    }

    return ret;
  }

  /*! \brief Get individual tile (as a reference)
   *
   * NOTE: from StackOverflow (recommended getter method):
   * Other_t& get_othertype(const std::string& name)
   * {
   *     auto it = otMap.find(name);
   *     if (it == otMap.end()) throw std::invalid_argument("entry not found");
   *     return *(it->second);
   * }
   *
   * This way map retains its ownership of the tile and we avoid giving pointers
   * away from the Class.
   */
  Tile_t& get_tile(const uint64_t cid) {
    auto it = tiles.find(cid);
    if (it == tiles.end()) { throw std::invalid_argument("tile entry not found"); }

    return *(it->second);
  }

  template<typename... Indices>
    corgi::internals::enable_if_t< (sizeof...(Indices) == D) && 
    corgi::internals::are_integral<Indices...>::value, 
  Tile_t&>
  get_tile_ind(const Indices... indices) {
    uint64_t cid = id(indices...);
    return get_tile(cid);
  }

  /// \brief Get individual tile (as a pointer)
  Tileptr get_tileptr(const uint64_t cid) {
    auto it = tiles.find(cid);
    //if (it == tiles.end()) { throw std::invalid_argument("entry not found"); }
    if (it == tiles.end()) { return nullptr; };
    return it->second;
  }

  template<typename... Indices>
    corgi::internals::enable_if_t< (sizeof...(Indices) == D) && 
    corgi::internals::are_integral<Indices...>::value, 
  Tileptr>
  get_tileptr_ind(const Indices... indices)
  {
    uint64_t cid = id(indices...);
    return get_tileptr(cid);
  }

  Tileptr get_tileptr(const std::tuple<size_t> ind) {
    size_t i = std::get<0>(ind);
    return get_tileptr_ind(i);
  }

  Tileptr get_tileptr(const std::tuple<size_t, size_t> ind) {
    size_t i = std::get<0>(ind);
    size_t j = std::get<1>(ind);
    return get_tileptr_ind(i, j);
  }

  Tileptr get_tileptr(const std::tuple<size_t, size_t, size_t> ind) {
    size_t i = std::get<0>(ind);
    size_t j = std::get<1>(ind);
    size_t k = std::get<2>(ind);
    return get_tileptr_ind(i, j, k);
  }


  /// Return all local tiles
  std::vector<uint64_t> get_local_tiles(
      const bool sorted=false ) {

    std::vector<uint64_t> tile_list = get_tile_ids(sorted);

    size_t i = 0, len = tile_list.size();
    while (i < len) {
      if (!tiles.at( tile_list[i] )->communication.local) {
        std::swap(tile_list[i], tile_list.back());
        tile_list.pop_back();
        len -= 1;
      } else {
        i++;
      }
    }

    return tile_list;
  }


  /// Return all tiles that are of VIRTUAL type.
  std::vector<uint64_t> get_virtuals(
      const bool sorted=false ) {
    std::vector<uint64_t> tile_list = get_tile_ids(sorted);

    size_t i = 0, len = tile_list.size();
    while (i < len) {
      if (tiles.at( tile_list[i] )->communication.local) {
        std::swap(tile_list[i], tile_list.back());
        tile_list.pop_back();
        len -= 1;
      } else {
        i++;
      }
    }

    return tile_list;
  }

  /// Return all local boundary tiles
  std::vector<uint64_t> get_boundary_tiles(
      const bool sorted=false ) {

    std::vector<uint64_t> tile_list = get_tile_ids(sorted);

    size_t i = 0, len = tile_list.size();
    while (i < len) {

      // remove if there are no virtual nbors and tile is not mine -> opposite means its boundary
      if (tiles.at( tile_list[i] )-> communication.number_of_virtual_neighbors == 0 || 
          tiles.at( tile_list[i] )-> communication.owner != comm.rank()
          ) {
        std::swap(tile_list[i], tile_list.back());
        tile_list.pop_back();
        len -= 1;
      } else {
        i++;
      }
    }

    return tile_list;
  }


  // /// Check if we have a tile with the given index
  bool is_local(uint64_t cid) {
    bool local = false;

    // Do we have it on the list=
    if (tiles.count( cid ) > 0) {
      // is it local (i.e., not virtual)
      if ( tiles.at(cid)->communication.local ) {
        local = true;
      }
    }

    return local;
  }


  /// return all virtual tiles around the given tile
  std::vector<int> virtual_nhood(uint64_t cid) {

    auto& c = get_tile(cid);
    auto neigs = c.nhood();
    //std::vector< corgi::internals::tuple_of<D, size_t> > neigs = c.nhood();
    std::vector<int> virtual_owners;
    for (auto& indx: neigs) {

      // Get tile id from index notation
      uint64_t cid = id(indx);

      if (!is_local( cid )) {
        int whoami = _mpi_grid(indx); 
        virtual_owners.push_back( whoami );
      }
    }

    return virtual_owners;
  }


  // /*! Analyze my local boundary tiles that will be later on
  //  * send to the neighbors as virtual tiles. 
  //  *
  //  * This is where the magic happens and we analyze what and who to send to.
  //  * These values *must* be same for everybody, this is why we use
  //  * mode of the owner list and in case of conflict pick the smaller value.
  //  * This way everybody knows what to expect and we avoid creating conflicts 
  //  * in communication. This information is then being sent to other processes 
  //  * together with the tiles and is analyzed there by others inside the
  //  * `rank_virtuals` function.
  //  * */
  void analyze_boundaries() {

    for (auto cid: get_local_tiles()) {
      std::vector<int> virtual_owners = virtual_nhood(cid);
      size_t N = virtual_owners.size();

      // If N > 0 then this is a boundary tile.
      // other criteria could also apply but here we assume
      // neighborhood according to spatial distance.
      if (N > 0) {

        /* Now we analyze `owner` vector as:
         * - sort the vector
         * - compute mode of the list to see who owns most of the
         * - remove repeating elements creating a unique list. */

        // sort
        std::sort( virtual_owners.begin(), virtual_owners.end() );

        // compute mode by creating a frequency array
        // NOTE: in case of same frequency we implicitly pick smaller rank
        int max=0, top_owner = virtual_owners[0];
        for(size_t i=0; i<virtual_owners.size(); i++) {
          int co = (int)count(virtual_owners.begin(), 
              virtual_owners.end(), 
              virtual_owners[i]);
          if(co > max) {      
            max = co;
            top_owner = virtual_owners[i];
          }
        } 

        // remove duplicates
        virtual_owners.erase( unique( virtual_owners.begin(), 
              virtual_owners.end() 
              ), virtual_owners.end() );


        // update tile values
        auto& c = get_tile(cid);
        c.communication.top_virtual_owner = top_owner;
        c.communication.communications    = virtual_owners.size();
        c.communication.number_of_virtual_neighbors = N;
        c.communication.virtual_owners = virtual_owners;

        if (std::find( send_queue.begin(), send_queue.end(),
              cid) == send_queue.end()
           ) {
          send_queue.push_back( cid );
          send_queue_address.push_back( virtual_owners );
        }
      }
    }
  }


  // /// Clear send queue, issue this only after the send has been successfully done
  void clear_send_queue() {
    send_queue.clear();
    send_queue_address.clear();
  }


  // // --------------------------------------------------
  // // Send queues etc.
  //   
  // /// list of tile id's that are to be sent to others
  std::vector<uint64_t> send_queue;

  // /// list containing lists to where the aforementioned send_queue tiles are to be sent
  std::vector< std::vector<int> > send_queue_address;


  // public:
  // // -------------------------------------------------- 

  std::vector<mpi::request> sent_info_messages;
  std::vector<mpi::request> sent_tile_messages;
  std::vector< std::vector<mpi::request> > sent_data_messages;

  std::vector<mpi::request> recv_info_messages;
  std::vector<mpi::request> recv_tile_messages;
  std::vector< std::vector<mpi::request> > recv_data_messages;


  // /// Broadcast master ranks mpi_grid to everybody
  void bcast_mpi_grid() {

    // total size
    int N = 1;
    for (size_t i = 0; i<D; i++) N *= _lengths[i];
    std::vector<int> tmp;

    if (comm.rank() == 0) {
      tmp = _mpi_grid.serialize();
    } else {
      tmp.resize(N);
      for(int k=0; k<N; k++) {tmp[k] = -1.0;};
    }

    MPI_Bcast(&tmp[0],
        N, 
        MPI_INT, 
        0, 
        MPI_COMM_WORLD
        );

    // unpack
    if(comm.rank() != 0) {
      _mpi_grid.deserialize(tmp, _lengths);
    }
  }


  /// Issue isends to everywhere
  // First we send a warning message of how many tiles to expect.
  // Based on this the receiving side can prepare accordingly.
  void send_tiles() {

    sent_info_messages.clear();
    sent_tile_messages.clear();

    for (int dest = 0; dest<comm.size(); dest++) {
      if( dest == comm.rank() ) { continue; } // do not send to myself

      int i = 0;
      std::vector<int> to_be_sent;
      for (std::vector<int> address: send_queue_address) {
        if( std::find( address.begin(),
              address.end(),
              dest) != address.end()) 
        {
          to_be_sent.push_back( i );
        }
        i++;
      }

      // initial message informing how many tiles are coming
      // TODO: this whole thing could be avoided by using 
      // MPI_Iprobe in the receiving end. Maybe...
      auto number_of_incoming_tiles = static_cast<int>(to_be_sent.size());

      //std::cout << comm.rank() 
      //          << " sending message to " 
      //          << dest
      //          << " incoming number of tiles " 
      //          << number_of_incoming_tiles
      //          << "\n";

      mpi::request req;
      req = comm.isend(dest, commType::NTILES, number_of_incoming_tiles);
      sent_info_messages.push_back( req );

    }

    // send the real tile meta info data now
    // We optimize this by only packing the tile data
    // once, and then sending the same thing to everybody who needs it.
    // FIXME: not really...
    int i = 0;
    for(auto cid: send_queue) {
      auto& tile = get_tile(cid);
      for(int dest: send_queue_address[i]) {

        mpi::request req;
        req = comm.isend(dest, commType::TILEDATA, tile.communication);

        sent_tile_messages.push_back( req );
      }
      i++;
    }

  }

  /// Send individual tile to dest
  void send_tile(uint64_t cid, int dest)
  {
    mpi::request req;

    auto& tile = get_tile(cid);
    //std::cout << comm.rank() << ": sending cid" << cid << "/" << tile.communication.cid << "\n";
    req = comm.isend(dest, 0, tile.communication);

    // FIXME and make non-blocking
    req.wait();

    return;
  }

  void recv_tile(int orig)
  {
    mpi::request req;

    Communication rcom;
    req = comm.irecv(orig, 0, rcom);

    // FIXME and make non-blocking
    req.wait();

    //std::cout << comm.rank() << ":"
    //  << "cid: " << rcom.cid
    //  << "ind: " << rcom.indices[0] << " " << rcom.indices[1] << " " << rcom.indices[2]
    //  << "owner: " << rcom.owner
    //  << "topo: " << rcom.top_virtual_owner
    //  << "comms: " << rcom.communications
    //  << "numv: " << rcom.number_of_virtual_neighbors
    //  << "mins: " << rcom.mins[0] << " " << rcom.mins[1] << " " << rcom.mins[2]
    //  << "maxs: " << rcom.maxs[0] << " " << rcom.maxs[1] << " " << rcom.maxs[2]
    //  << "\n";

    // next need to build tile
    rcom.local = false; // received tiles are automatically virtuals
    create_tile(rcom);

    return;
  }

  /// Receive incoming stuff
  void recv_tiles() {

    recv_info_messages.clear();
    recv_tile_messages.clear();

    size_t i = 0;
    for (int source=0; source<comm.size(); source++) {
      if (source == comm.rank() ) continue; // do not receive from myself

      // communicate with how many tiles there are incoming

      // TODO: use MPI_IProbe to check if there are 
      // any messages for me instead of assuming that there is

      // TODO: encapsulate into vector that can be received & 
      // processed more later on

      int number_of_incoming_tiles;
      mpi::request req;
      req = comm.irecv(source, commType::NTILES, number_of_incoming_tiles);

      // TODO: Remove this code block and do in background instead
      req.wait();
      recv_info_messages.push_back( req );

      /*
         fmt::print("{}: I got a message! Waiting {} tiles from {}\n",
         rank, number_of_incoming_tiles, source);
         */
      //std::cout << comm.rank()
      //          << " I got a message! Waiting " 
      //          << number_of_incoming_tiles << " tiles from " 
      //          << source
      //          << "\n";

      // Now receive the tiles themselves
      size_t j = recv_tile_messages.size();
      for (int ic=0; ic<number_of_incoming_tiles; ic++) {
        mpi::request reqc;
        Communication rcom;

        reqc = comm.irecv(source, commType::TILEDATA, rcom);
        
        // TODO non blocking
        reqc.wait();
        recv_tile_messages.push_back( reqc );
          
        j++;

        if(this->tiles.count(rcom.cid) == 0) { // Tile does not exist yet; create it

          // TODO: Check validity of the tile better
          rcom.local = false; // received tiles are automatically virtuals
          create_tile(rcom);

        } else { // Tile is already on my virtual list; update
          update_tile(rcom);  
        };
      }
      i++;
    }
  }

  /// Initialize vector of vector if needed
  void initialize_message_array(
      std::vector<std::vector<mpi::request>>& arr, 
      int tag)
  {
    while((int)arr.size() <= tag) {
      std::vector<mpi::request> arri(0);
      arr.push_back( arri );
    }
  }

  /// Call mpi send routines from tile for the boundary regions
  // NOTE: we bounce sending back to tile members,
  //       this way they can be extended for different types of send.
  void send_data(int tag)
  {
    initialize_message_array(sent_data_messages, tag);
    sent_data_messages.at(tag).clear();

    for(auto cid : get_boundary_tiles() ) {
      auto& tile = get_tile(cid);
      for(auto dest: tile.communication.virtual_owners) {
        auto reqs = tile.send_data(comm, dest, tag);
        
        for(auto& req : reqs) sent_data_messages.at(tag).push_back(req);
      }
    }
  }


  /// Call mpi recv routines from tile for the virtual regions
  // NOTE: we bounce receiving back to tile members,
  //       this way they can be extended for different types of recv.
  void recv_data(int tag)
  {
    initialize_message_array(recv_data_messages, tag);
    recv_data_messages.at(tag).clear();

    for(auto cid : get_virtuals() ) {
      auto& tile = get_tile(cid);
      auto reqs = tile.recv_data(comm, tile.communication.owner, tag);

      for(auto& req : reqs) recv_data_messages.at(tag).push_back(req);
    }
  }

  /// barrier until all (primary) data is received
  void wait_data(int tag)
  {
    assert( tag < (int)recv_data_messages.size() );
    mpi::wait_all(recv_data_messages[tag].begin(), recv_data_messages[tag].end());
  }



}; // end of Node class

} // end of corgi namespace


