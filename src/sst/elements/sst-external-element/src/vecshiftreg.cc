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

#include <sst/core/sst_config.h>
#include "sst/core/event.h"
#include "vecshiftreg.h"
#include "RTLEvent.h"
#include "VecShiftRegister_O1.h"
#include "arielrtlev.h"

using namespace SST;
using namespace SST::VecShiftReg;

vecShiftReg::vecShiftReg(ComponentId_t id, Params& params) :
	Component(id) {

    bool found;
    dut = new VecShiftRegister;
    ev = new RTLEvent;
	output.init("vecShiftReg-" + getName() + "-> ", 1, 0, SST::Output::STDOUT);

	RTLClk  = params.find<std::string>("ExecFreq", 0, found);
    if (!found) {
        Simulation::getSimulation()->getSimulationOutput().fatal(CALL_INFO, -1,"couldn't find work per cycle\n");
    }

	maxCycles = params.find<Cycle_t>("maxCycles", 0, found);
    if (!found) {
        Simulation::getSimulation()->getSimulationOutput().fatal(CALL_INFO, -1,"couldn't find work per cycle\n");
    }

	/*if(RTLClk == NULL || RTLClk == "0") 
		output.fatal(CALL_INFO, -1, "Error: printFrequency must be greater than zero.\n");*/

	output.verbose(CALL_INFO, 1, 0, "Config: maxCycles=%" PRIu64 ",RTL Clock Freq=%s\n",
		static_cast<uint64_t>(maxCycles), static_cast<std::string>(RTLClk));

	// Just register a plain clock for this simple example
   output.verbose(CALL_INFO, 1, 0, "Registering RTL clock at %s\n", RTLClk.c_str());
   
   /*registerExit();*/

   //set our clock 
   TimeConverter* timeconverter = registerClock(RTLClk, new Clock::Handler<vecShiftReg>(this, &vecShiftReg::clockTick));

    //Configure and register Event Handler for Rtllink
   RtlLink = configureLink("Rtllink", new Event::Handler<vecShiftReg>(this, &vecShiftReg::handleEvent)); 

   // Tell SST to wait until we authorize it to exit
   registerAsPrimaryComponent();
   primaryComponentDoNotEndSim();

   assert(RtlLink);
}

vecShiftReg::~vecShiftReg() {
    delete ev;
    delete dut;
}

void vecShiftReg::setup() {
    dut->reset = UInt<1>(1);
	output.verbose(CALL_INFO, 1, 0, "Component is being setup.\n");
    
}

//Nothing to add in finish as of now. Need to see what could be added.
void vecShiftReg::finish() {
	output.verbose(CALL_INFO, 1, 0, "Component is being finished.\n");
}

//clockTick will actually execute the RTL design at every cycle based on the input and control signals updated by Ariel CPU or Event Handler.
bool vecShiftReg::clockTick( SST::Cycle_t currentCycle ) {

    /*uint64_t DUTclk; 
    char* endPtr;
    DUTclk = strtoull(RTLClk.c_str(), &endPtr, 10);
    DUTcycles = 10^9/DUTclk;
	unitAlgebra clock_ua(RTLClk);
    if( currentCycle %  == 0 ) {
		output.verbose(CALL_INFO, 1, 0, "Hello World!\n");
	}*/
    dut->eval(update_registers, verbose, done_reset);
    
	tickCount++;

	if( tickCount >= dynCycles || tickCount >= maxCycles ) {
        if(sim_done) {
            primaryComponentOKToEndSim();  //Tell the SST that it can finish the simulation.
            /*unregisterExit();*/  
            return true;
        }
	} else {
		return false;
	}
}

/*Event Handle will be called by Ariel CPU once it(Ariel CPU) puts the input and control signals in the shared memory. Now, we need to modify Ariel CPU code for that.
Event handler will update the input and control signal based on the workload/C program to be executed.
Don't know what should be the argument of Event handle as of now. As, I think we don't need any argument. It's just a requst/call by Ariel CPU to update input and control signals.*/
void vecShiftReg::handleEvent(SST::Event *event) {
    /*
    * Event will pick information from shared memory. (What will be the use of Event queue.)
    * Need to insert code for it. 
    * Probably void pointers will be used to get the data from the shared memory which will get casted based on the width set by the user at runtime.
    * void pointers will be defined by Ariel CPU and passed as parameters through SST::Event to the C++ model. 
    * As of now, shared memory is like a scratch-pad or heap which is passive without any intelligent performance improving stuff like TLB, Cache hierarchy, accessing mechanisms(VIPT/PIPT) etc.  
    */
    ArielRtlEvent* ariel_ev = dynamic_cast<ArielRtlEvent*>event;

    bool* updated_rtl_params = (bool)ariel_ev->get_updated_rtl_params();

    update_inp = *updated_rtl_params;
    update_ctrl = *updated_rtl_params++;
    update_eval_args = *updated_rtl_params++;
    uint64_t* dynCycles_ptr = (uint64_t*)(++updated_rtl_params);
    dynCycles = *dynCycles_ptr;
    updated_rtl_params = (bool*)(++dynCycles_ptr);
    
    sim_done = *updated_rtl_params;

    if(update_inp) {
        ev->inp_ptr = ariel_ev->get_rtl_inp_ptr(); 
        ev->inp_info = ariel_ev->get_rtl_inp_info();
        ev->input_sigs(dut);
    }

    if(update_ctrl) {
        ev->ctrl_ptr = ariel_ev->get_rtl_ctrl_ptr(); 
        ev->ctrl_info = ariel_ev->get_rtl_ctrl_info();
        ev->control_sigs(dut);
    }

    if(update_eval_args) {
        update_registers = *updated_rtl_params++;
        verbose = *updated_rtl_params++;
        done_reset = *updated_rtl_params++;
    }
    //loading initial values to the RTL design
    dut->eval(update_registers, verbose, done_reset);

}
