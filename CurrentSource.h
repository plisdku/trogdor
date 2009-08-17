/*
 *  CurrentSource.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/14/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _CURRENTSOURCE_
#define _CURRENTSOURCE_

#include <vector>
#include "MemoryUtilities.h"
#include "SimulationDescription.h"

/*
    Let's think about this now for a sec.  What does the current source do?
    Well, first off, it is included in the update equation by a template
    mechanism, which will rapidly increase the size of the executable if I
    am not careful.  So whatever actually gets thrown in with the templates
    will have to be a *very good* piece of code, or it will need to use a
    virtual function call.
    
    This is a rather unhappy circumstance.  One way to solve the problem would
    be to concoct a buffering mechanism that will let me feed the update
    equation efficiently from any source; for instance, I could assume that
    the source data is always in a buffer, and increment the buffer pointer
    by one or by zero depending on whether the source is space-varying or not.
    The trouble is that the buffer may need to be precomputed somehow.
    
    A decent solution would pre-allocate the buffer to the length of the longest
    runline, then load it completely in advance per runline.
    
    There are in any case two distinct objects here, since each UpdateEquation
    will have its own current source object to feed J.
    
    I suppose I can be happy with a buffer that holds space-varying currents.
    This way it'll run fast; the loading can be done per material per timestep,
    somehow.
    
    
    
    How does this work?
    Description: must paint it into the grid at assembly
    Setup: 
*/

class VoxelizedPartition;
class CalculationPartition;
class CurrentSource;

class SetupCurrentSource
{
public:
    SetupCurrentSource(const CurrentSourceDescPtr & description);
    virtual ~SetupCurrentSource();
    
    virtual Pointer<CurrentSource> makeCurrentSource(
        const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
        
    
    CurrentSourceDescPtr getDescription() const { return mDescription; }
private:
    CurrentSourceDescPtr mDescription;
};
typedef Pointer<SetupCurrentSource> SetupCurrentSourcePtr;


class CurrentSource
{
public:
    CurrentSource();
    virtual ~CurrentSource();
    
    virtual void prepareJ(long timestep);
    virtual void prepareK(long timestep);
    
    virtual float getJ(int direction) const;
    virtual float getK(int direction) const;
private:
    
};
typedef Pointer<CurrentSource> CurrentSourcePtr;

#endif
