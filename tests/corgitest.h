#pragma once

#include <iostream>

#include "../corgi.h"


namespace corgitest {

class Welsh : public corgi::Tile<2> {

  public:
    Welsh(size_t i, size_t j, 
             int o, 
             size_t nx, size_t ny
             ) : corgi::Tile<2>(i, j, o, nx, ny) { }

    ~Welsh() override = default;

    // extend the base class
    std::string bark();

};


class Pembroke : public corgi::Tile<2> {

  public:
    Pembroke(size_t i, size_t j, 
             int o, 
             size_t nx, size_t ny
             ) : corgi::Tile<2>(i, j, o, nx, ny) { }

    ~Pembroke() override = default;

    // extend the base class
    std::string bark();

    // specialize the this class
    std::string howl();

};


//-------------------------------------------------- 
// Following definitions introduce a more complex class
// called Vallhund which uses multiple inheritance from 
// several classes.

/// Viking class 
class Viking {

  public:
    Viking() { std::cout << "...making a Viking()! \n"; };

    /// special method only known to real Vikings
    std::string prayForOdin();
};


/// Swede class
class Swede {
  
  public:

    /// Swedish social security number
    int number;


    /// special constructor with a social sec. number
    Swede(int n) : number(n)
    { 
      std::cout << "...making a Swede()";
      std::cout << " with a social security number " << number << "\n";
    };
      
    /// Special method only known to Swedes
    std::string fika();

};


/// \brief Swedish Vallhund is a special corgi breed with hint of Viking in it
//
//  It has a hint of Viking blood in it, in addition to
//  some Swedish peculiarities.  We use multiple inheritance 
//  to inherit special methods from Viking class and from Swede 
//  class.
//
class Vallhund : public Swede, 
                 public Viking, 
                 public corgi::Tile<2> {

  public:
                   
    Vallhund(size_t i, size_t j, 
             int o, 
             size_t nx, size_t ny,
             int social_number
             ) : 
      Swede(social_number),
      corgi::Tile<2>(i, j, o, nx, ny) 
    { }

    ~Vallhund() override = default;


    std::string bark();

};



class Grid : public corgi::Node<2> {

  public:
    Grid(size_t nx, size_t ny) : corgi::Node<2>(nx, ny) { }

    ~Grid() = default;

    // std::unordered_map< uint64_t, std::shared_ptr<corgi::Tile>> cells;

    std::string petShop();


};



} // end of namespace corgitest