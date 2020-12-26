// Copyright 2009-2020 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2020, NTESS
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef _SIMPLE_VECTORSHIFTREG_H
#define _SIMPLE_VECTORSHIFTREG_H

#include <sst/core/component.h>
#include "VecShiftRegister_O1.h"
#include <sst/core/link.h>
#include <sst/core/rng/marsaglia.h>
#include <sst/core/eli/elementinfo.h>
#include "RTLEvent.h"

namespace SST {
namespace VecShiftReg {

class vecShiftReg : public SST::Component {

public:
	vecShiftReg( SST::ComponentId_t id, SST::Params& params );
	~vecShiftReg();

	void setup();
	void finish();

	bool clockTick( SST::Cycle_t currentCycle );

	SST_ELI_REGISTER_COMPONENT(
		vecShiftReg,
		"vecshiftreg",
		"vecShiftReg",
		SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
		"Demonstration of an External Element for SST",
		COMPONENT_CATEGORY_PROCESSOR
	)
    //Stats needs to be added. Now, stats will be added based on the outputs as mentioned by the user based on the RTL login provided. 
    SST_ELI_DOCUMENT_STATISTICS(
    )
    //Parameters will mostly be just frequency/clock in the design. User will mention specifically if there could be other parameters for the RTL design which needs to be configured before runtime.  Don't mix RTL input/control signals with SST parameters. SST parameters of RTL design will make the RTL design/C++ model synchronous with rest of the SST full system.   
	SST_ELI_DOCUMENT_PARAMS(
		{ "ExecFreq", "Clock frequency of RTL design in GHz", "1GHz" },
		{ "maxCycles", "Number of Clock ticks the simulation must atleast execute before halting", "1000" },
	)

    //Default will be single port for communicating with Ariel CPU. Need to see the requirement/use-case of multi-port design and how to incorporate it in our parser tool.  
    SST_ELI_DOCUMENT_PORTS(
        {"Rtllink", "Link to the vecShiftReg", { "vecShiftReg.vecShiftRegEvent", "" } }
    )


private:
	SST::Output output;
    
    //RTL Clock frequency of execution and maximum Cycles/clockTicks for which RTL simulation will run.
    std::string RTLClk;
	SST::Cycle_t maxCycles;

    void handleEvent(SST::Event *ev);
    SST::Link* RtlLink;
    
    bool update_registers, verbose, done_reset, sim_done;
    bool update_inp, update_ctrl, update_eval_args;
    RTLEvent *ev;
    VecShiftRegister *dut;

    uint64_t tickCount;
    uint64_t dynCycles; 
};

} // namespace VecShiftReg
} // namespace SST

#endif //_SIMPLE_VECSHIFTREG_H
