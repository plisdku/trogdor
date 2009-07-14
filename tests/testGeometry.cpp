// testmain.cpp

// these two defines tell Boost to provide a main() function.  GRRRRR WHY
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Yeah yeah

// my main() is defined in boost/test/unit_test.hpp.
#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#include "geometry.h"
#include <iostream>

// Two tasks may need to be carried out before testing:
//  1. The test tree needs to be built; alternatively I can use automated
//      unit test registration (like BOOST_AUTO_TEST_CASE?)
//  2.  Custom test module initialization.  This includes initialization of 
//      the code under test as needed; it also includes some boost.test things
//      like redirection of output streams, according to the documentation...
//
//  For many test modules, no tree is necessary.  (This is a clue about how
//  large the scope of a "module" ought to be.  This documentation is awful!)

// I can either perform test setup and teardown by implementing a function
// (the "test module initialization function") or, in a more granular way
// (maybe) using "fixtures" which are a sort of unified pattern for setup and
// teardown.  A fixture is just a struct with a constructor and destructor that
// perform setup and teardown.  Nice.
//

// REALITY: EITHER
// -- Use "global fixtures"
// -- write one of two initialization functions (prolly don't need to bother)
// -- define BOOST_TEST_MAIN (my approach) to do the default initialization 

BOOST_AUTO_TEST_CASE(RectSingularDimensions)
{
    BOOST_REQUIRE(Rect3i(0,0,0,10,0,0).numNonSingularDims() == 1);
    BOOST_REQUIRE(Rect3i(0,0,0,0,0,10).numNonSingularDims() == 1);
    BOOST_REQUIRE(Rect3i(0,0,0,0,10,0).numNonSingularDims() == 1);
}

BOOST_AUTO_TEST_CASE(SillyVectorTest)
{
    Vector3i v(0,0,0), w(1,1,1);
    BOOST_REQUIRE(w - v == Vector3i(1,1,1));
    
    /*BOOST_CHECK(true);
    BOOST_CHECK(false);
    BOOST_REQUIRE(1 == 0);
    BOOST_REQUIRE(false);*/
}


