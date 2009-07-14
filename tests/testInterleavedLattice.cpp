// Test interleaved lattice.cpp

// these two defines tell Boost to provide a main() function.  GRRRRR WHY
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Test InterleavedLattice

// my main() is defined in boost/test/unit_test.hpp.
#include <boost/test/unit_test.hpp>

#include "InterleavedLattice.h"
#include "YeeUtilities.h"
#include <iostream>
#include <string>

using namespace YeeUtilities;
using namespace std;

static const int L = 10;

class TestData
{
public:
    TestData();
    
    ~TestData()
    {
        BOOST_TEST_MESSAGE("Tearing down.");
    }
    
    InterleavedLatticePtr lattice1d;
    InterleavedLatticePtr lattice2d;
    InterleavedLatticePtr lattice3d;
};

TestData::
TestData()
{
    //BOOST_TEST_MESSAGE("Setting up.");
    lattice1d = InterleavedLatticePtr(new InterleavedLattice(
        string("1D"), Rect3i(0,0,0,L-1,1,1)));
    lattice2d = InterleavedLatticePtr(new InterleavedLattice(
        string("2D"), Rect3i(-L+1, -L+1, 0, 0, 0, 1)));
    lattice3d = InterleavedLatticePtr(new InterleavedLattice(
        string("3D"), Rect3i(0,0,0,2*L-1,2*L-1,2*L-1)));
}

BOOST_AUTO_TEST_CASE(checkLIsEven)
{
    BOOST_CHECK_EQUAL( (L/2)*2, L );
}

BOOST_FIXTURE_TEST_CASE(wrap, TestData)
{
    Vector3i halfCell1(L+2,2,4);
    Vector3i halfCell2(L+3,1,5);
    Vector3i halfCell3(-1,0,5*L+1);
    
    BOOST_CHECK_EQUAL(lattice1d->halfCells().size(), Vector3i(L-1,1,1));
    BOOST_CHECK_EQUAL(lattice1d->wrap(halfCell1), Vector3i(2,0,0));
    BOOST_CHECK_EQUAL(lattice1d->wrap(halfCell2), Vector3i(3,1,1));
    BOOST_CHECK_EQUAL(lattice1d->wrap(halfCell3), Vector3i(L-1,0,1));
    
    BOOST_CHECK_EQUAL(lattice2d->halfCells().size(), Vector3i(L-1,L-1,1));
    BOOST_CHECK_EQUAL(lattice2d->wrap(halfCell1), Vector3i(-L+2, -L+2, 0));
    BOOST_CHECK_EQUAL(lattice2d->wrap(halfCell2), Vector3i(-L+3, -L+1, 1));
    BOOST_CHECK_EQUAL(lattice2d->wrap(halfCell3), Vector3i(-1,0,1));
    
    BOOST_CHECK_EQUAL(lattice3d->halfCells().size(),
        Vector3i(2*L-1,2*L-1,2*L-1));
    BOOST_CHECK_EQUAL(lattice3d->wrap(halfCell1), Vector3i(L+2,2,4));
    BOOST_CHECK_EQUAL(lattice3d->wrap(halfCell2), Vector3i(L+3,1,5));
    BOOST_CHECK_EQUAL(lattice3d->wrap(halfCell3), Vector3i(2*L-1,0,L+1));
}

BOOST_AUTO_TEST_CASE(setAndGet)
{
    Vector3i v;
    InterleavedLattice l(string("Temp"), Rect3i(0,0,0,L-1,L-1,L-1));
    l.allocate();
    
    for (int fieldDir = 0; fieldDir < 3; fieldDir++)
    {        
        for (v[2] = 0; v[2] < L/2; v[2]++)
        for (v[1] = 0; v[1] < L/2; v[1]++)
        for (v[0] = 0; v[0] < L/2; v[0]++)
        {
            l.setE(fieldDir, v, fieldDir+100*v[2] + 10000*v[1] + 1000000*v[0]);
            l.setH(fieldDir, v, -(fieldDir+100*v[2] + 10000*v[1] + 1000000*v[0]));
        }
    }
    for (int fieldDir = 0; fieldDir < 3; fieldDir++)
    {
        for (v[2] = 0; v[2] < L/2; v[2]++)
        for (v[1] = 0; v[1] < L/2; v[1]++)
        for (v[0] = 0; v[0] < L/2; v[0]++)
        {
            BOOST_CHECK_EQUAL(l.getE(fieldDir, v),
                fieldDir+100*v[2] + 10000*v[1] + 1000000*v[0]);
            
            BOOST_CHECK_EQUAL(l.getH(fieldDir, v),
                -(fieldDir+100*v[2] + 10000*v[1] + 1000000*v[0]) );
        }
        
        BOOST_CHECK_EQUAL(l.getWrappedE(fieldDir, Vector3i(L/2, L/2, L/2)),
            l.getE(fieldDir, Vector3i(0,0,0)));
        BOOST_CHECK_EQUAL(l.getWrappedH(fieldDir, Vector3i(L/2+4, L/2, 0)),
            l.getH(fieldDir, Vector3i(4, 0, 0)));
        BOOST_CHECK_EQUAL(l.getWrappedH(fieldDir, Vector3i(4,0,0)),
            l.getH(fieldDir, Vector3i(4, 0, 0)));
    }
}

BOOST_AUTO_TEST_CASE(pointerSetterGetter)
{
    Vector3i v;
    for (int fieldDir = 0; fieldDir < 3; fieldDir++)
    {
        InterleavedLattice l(string("Temp"), Rect3i(0,0,0,L-1,L-1,L-1));
        l.allocate();
        
        for (v[2] = 0; v[2] < L/2; v[2]++)
        for (v[1] = 0; v[1] < L/2; v[1]++)
        for (v[0] = 0; v[0] < L/2; v[0]++)
        {
            BufferPointer ePtr = l.pointerE(fieldDir, v);
            *(ePtr.getPointer()) = v[2] + 100*v[1] + 10000*v[0];
            
            BufferPointer hPtr = l.pointerH(fieldDir, v);
            *(hPtr.getPointer()) = -( v[2] + 100*v[1] + 10000*v[0] );
            
            BOOST_CHECK_EQUAL(*(ePtr.getPointer()),
                l.getE(fieldDir, v));
            
            BOOST_CHECK_EQUAL(*(hPtr.getPointer()),
                l.getH(fieldDir, v));
        }
    }
}

BOOST_AUTO_TEST_CASE(halfCellPtr)
{
    Vector3i v;
    for (int fieldDir = 0; fieldDir < 3; fieldDir++)
    {
        InterleavedLattice l(string("Temp"), Rect3i(0,0,0,L-1,L-1,L-1));
        l.allocate();
        
        for (v[2] = 0; v[2] < L/2; v[2]++)
        for (v[1] = 0; v[1] < L/2; v[1]++)
        for (v[0] = 0; v[0] < L/2; v[0]++)
        {
            BufferPointer ePtr = l.pointer(yeeToHalf(v, octantE(fieldDir)));
            *(ePtr.getPointer()) = v[2] + 100*v[1] + 10000*v[0];
            BufferPointer hPtr = l.pointer(yeeToHalf(v, octantH(fieldDir)));
            *(hPtr.getPointer()) = -( v[2] + 100*v[1] + 10000*v[0] );
                
            BOOST_CHECK_EQUAL(*(ePtr.getPointer()),
                l.getE(fieldDir, v));
            
            BOOST_CHECK_EQUAL(*(hPtr.getPointer()),
                l.getH(fieldDir, v));
        }
    }
}

BOOST_AUTO_TEST_CASE(using3dstride)
{
    Vector3i v;
    for (int fieldDir = 0; fieldDir < 3; fieldDir++)
    {
        InterleavedLattice l(string("Temp"), Rect3i(0,0,0,L-1,L-1,L-1));
        l.allocate();
        
        for (v[2] = 0; v[2] < L/2; v[2]++)
        for (v[1] = 0; v[1] < L/2; v[1]++)
        for (v[0] = 0; v[0] < L/2; v[0]++)
        {
            l.setE(fieldDir, v, v[2] + 100*v[1] + 10000*v[0]);
            l.setH(fieldDir, v, -(v[2] + 100*v[1] + 10000*v[0]));
        }
        
        Vector3i centerPt(L/4, L/4, L/4);
        Vector3i stride(l.fieldStride());
        
        BufferPointer p0e = l.pointerE(fieldDir, centerPt);
        BufferPointer p0h = l.pointerH(fieldDir, centerPt);
        
        BOOST_CHECK_EQUAL(*(p0e.getPointer()), l.getE(fieldDir, centerPt));
        BOOST_CHECK_EQUAL(*(p0e.getPointer()+stride[0]),
            l.getE(fieldDir, centerPt+Vector3i(1,0,0)));
        BOOST_CHECK_EQUAL(*(p0e.getPointer()+stride[1]),
            l.getE(fieldDir, centerPt+Vector3i(0,1,0)));
        BOOST_CHECK_EQUAL(*(p0e.getPointer()+stride[2]),
            l.getE(fieldDir, centerPt+Vector3i(0,0,1)));
            
        BOOST_CHECK_EQUAL(*(p0h.getPointer()), l.getH(fieldDir, centerPt));
        BOOST_CHECK_EQUAL(*(p0h.getPointer()+stride[0]),
            l.getH(fieldDir, centerPt+Vector3i(1,0,0)));
        BOOST_CHECK_EQUAL(*(p0h.getPointer()+stride[1]),
            l.getH(fieldDir, centerPt+Vector3i(0,1,0)));
        BOOST_CHECK_EQUAL(*(p0h.getPointer()+stride[2]),
            l.getH(fieldDir, centerPt+Vector3i(0,0,1)));
    }
}

// This is my first regression test! july 10 '09
BOOST_AUTO_TEST_CASE(zeroStride)
{
    InterleavedLattice l1d(string(),Rect3i(0,0,0,11,1,1));
    
    BOOST_CHECK_EQUAL(l1d.fieldStride()[1], 0);
    BOOST_CHECK_EQUAL(l1d.fieldStride()[2], 0);
}

BOOST_AUTO_TEST_CASE(yeeCellCalculations)
{
    for (int fieldDir = 0; fieldDir < 3; fieldDir++)
    {
        InterleavedLattice l(string("Temp"), Rect3i(0,0,0,L-1,L-1,L-1));
        l.allocate();
        
        Vector3i centerYee(L/4, L/4, L/4);
        
        l.setE(fieldDir, centerYee, 1.0);
        l.setH(fieldDir, centerYee, -1.0);
        
        BOOST_REQUIRE(l.halfCells().encloses(
            yeeToHalf(centerYee, octantE(fieldDir))));
        BOOST_REQUIRE(l.halfCells().encloses(
            yeeToHalf(centerYee, octantH(fieldDir))));
        
        BufferPointer pE = l.pointer(yeeToHalf(centerYee, octantE(fieldDir)));
        BufferPointer pH = l.pointer(yeeToHalf(centerYee, octantH(fieldDir)));
        
        BOOST_CHECK_EQUAL(*(pE.getPointer()), 1.0);
        BOOST_CHECK_EQUAL(*(pE.getPointer()), l.getE(fieldDir, centerYee));
        BOOST_CHECK_EQUAL(*(pH.getPointer()), -1.0);
        BOOST_CHECK_EQUAL(*(pH.getPointer()), l.getH(fieldDir, centerYee));
    }
}




