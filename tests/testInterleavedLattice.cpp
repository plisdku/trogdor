// testmain.cpp

// these two defines tell Boost to provide a main() function.  GRRRRR WHY
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE MyClass test

// my main() is defined in boost/test/unit_test.hpp.
#include <boost/test/unit_test.hpp>
using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#include "MyClass.h"
#include <iostream>
#include <cassert>

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


BOOST_AUTO_TEST_SUITE( Suite1 )

BOOST_AUTO_TEST_CASE(AnotherTest)
{
    assert(false);
    BOOST_REQUIRE(false);
}

BOOST_AUTO_TEST_CASE(Test1)
{
    BOOST_CHECK(true);
    BOOST_CHECK(false);
    BOOST_REQUIRE(1 == 0);
    BOOST_REQUIRE(false);
}

BOOST_AUTO_TEST_SUITE_END()


/*
class TestClass
{
public:
    TestClass() {}
    
    void testFive()
    {
        BOOST_REQUIRE(mTestMe.returnFive() == 5);
    }
    
    void testSix()
    {
        BOOST_REQUIRE(mTestMe.returnSix() == 6);
    }
private:
    MyClass mTestMe;
};

class TestSuite : public test_suite
{
public:
   TestSuite() : test_suite("Test Suite A: Hypernion Quatarnian II: The Matrix")
   {
        // create an instance of the test cases class
        boost::shared_ptr<TestClass> instance(new TestClass());
        
        // create the test cases
        test_case* fiveTestCase = BOOST_CLASS_TEST_CASE(
          &TestClass::testFive, instance );
        test_case* sixTestCase = BOOST_CLASS_TEST_CASE(
          &TestClass::testSix, instance );
        
        // add the test cases to the test suite
        add(fiveTestCase);
        add(sixTestCase);
   }
private:
};
*/

// Check it out: if I define BOOST_TEST_ALTERNATIVE_API then I use
// bool init_test_func()
// else I use
// test_suite* init_unit_test_suite(int argc, char** argv).
// Happily this is ALL AVOIDABLE if I define BOOST_TEST_MAIN, which I do.
// THIS DOCUMENTATION SUCKS ARRGGGGHHH WHY IS IT SO BAD!!!!!!!!111


/*
// The original unit test framework (UTF) required the programmer (me!) to
// implement this function as the test program entry point.  The return value
// USED to be the master test suite.  However the master suite is now managed
// by the UTF itself, and it's actually recommended now to just return NULL from
// this function and use the regular test suite add interface (whatever that
// is!) to add tests to that master suite.
// Although this function is a way to access the command-line arguments, they
// can also be reached through the UTF's master test suite facilities.
test_suite* init_unit_test_suite(int argc, char** argv)
{
    // create the top test suite
    test_suite* top_test_suite(BOOST_TEST_SUITE("Master test suite"));
    
    // add test suites to the top test suite (uh, two of them... identical...)
    top_test_suite->add(new TestSuite());
    top_test_suite->add(new TestSuite());
    
    return top_test_suite;
}

// The better way, using the master suite, is

test_suite* init_unit_test_suite(int argc, char** argv)
{
    framework::master_test_suite().
        add( BOOST_TEST_CASE( &free_test_function ) );
    return 0;
}

*/

// The alternative initialization (alternative to init_unit_test_suite) is to 
// implement this little guy, init_unit_test().  Return true if init worked.
// I kinda think this is the alternative to fixtures: either do setup and
// teardown with fixtures or with this function.
/*
bool init_unit_test()
{
}
*/

/*
int main(int argc, char** argv)
{
    MyClass mine;
    
    bool success = 1;
    
    if (mine.returnFive() != 5)
        success = 0;
    if (mine.returnSix() != 6)
        success = 0;
    
    if (success)
        std::cout << "Yay\n";
    
    return 1;
    return !success; // that's how it's done
}
*/