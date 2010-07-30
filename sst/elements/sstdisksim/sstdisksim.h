// Copyright 2009-2010 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
// 
// Copyright (c) 2009-2010, Sandia Corporation
// All rights reserved.
// 
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef _SSTDISKSIM_H
#define _SSTDISKSIM_H

#include "sstdisksim_event.h"

#include <sst/core/log.h>
#include <sst/core/component.h>
#include <sst/core/simulation.h>
#include <stdlib.h>
#include <stddef.h>
#include <sst/core/timeConverter.h>

#include "syssim_driver.h"
#include <disksim_interface.h>
#include <disksim_rand48.h>

using namespace std;
using namespace SST;

#ifndef DISKSIM_DBG
#define DISKSIM_DBG 0
#endif

typedef	struct	{
  int n;
  double sum;
  double sqr;
} sstdisksim_stat;

class sstdisksim : public Component {

 public:

  sstdisksim( ComponentId_t id, Params_t& params );
  ~sstdisksim();
  int Setup();
  int Finish();

  SysTime __now;		/* current time */
  SysTime __next_event;	/* next event */
  sstdisksim_stat __st;
  
 private:

  bool __done;
  sstdisksim_stat __disksim_stat;
  struct disksim_interface* __disksim;
  Simulation* __sim;
  Params_t __params;
  ComponentId_t __id;

  /* To be removed later-this is just to test the component
     before we start having trace-reading functionality. */
  int numsectors;

  Log< DISKSIM_DBG >&  m_dbg;
  
  sstdisksim( const sstdisksim& c );
  
  unsigned long sstdisksim_process_event(sstdisksim_event* ev);
  unsigned long processBlock(unsigned long blkno,
			     unsigned long count,
			     int devno, 
			     eventtype etype);
  void handleEvent(Event* ev);
  void emptyEvent(Event* ev);

  bool clock(Cycle_t current);


  SST::Link* link;
  SST::Link* empty;
  
  friend class boost::serialization::access;
  template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
      ar & BOOST_SERIALIZATION_NVP(numsectors);
      ar & BOOST_SERIALIZATION_NVP(link);
    }
  
  template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
      ar & BOOST_SERIALIZATION_NVP(numsectors);
      ar & BOOST_SERIALIZATION_NVP(link);
      ar & BOOST_SERIALIZATION_NVP(empty);

      
      link->setFunctor(new SST::Event::Handler<sstdisksim>(this, &sstdisksim::handleEvent));
      empty->setFunctor(new SST::Event::Handler<sstdisksim>(this, &sstdisksim::emptyEvent));
    }
	
  BOOST_SERIALIZATION_SPLIT_MEMBER()    
};

#endif /* _SSTDISKSIM_H */
