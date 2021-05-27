// Copyright 2009-2020 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2020, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#include "uint.h"
#include "sint.h"
#include "VecShiftRegister_O1.h"
#include "rtlevent.h"

using namespace SST;
using namespace SST::RtlComponent;

void RTLEvent::input_sigs(VecShiftRegister* dut) {

    
    dut->reset = UInt<1>(1);
    void* rtl_inp_ptr = inp_ptr;
    {
        UInt<4>* temp_ptr = (UInt<4>*)rtl_inp_ptr;
        dut->io_ins_0 = *temp_ptr;
        rtl_inp_ptr = ++temp_ptr;
    }

    {    
        UInt<4>* temp_ptr = (UInt<4>*)rtl_inp_ptr;
        dut->io_ins_1 = *temp_ptr;
        rtl_inp_ptr = ++temp_ptr;
    }

    {    
        UInt<4>* temp_ptr = (UInt<4>*)rtl_inp_ptr;
        dut->io_ins_2 = *temp_ptr;
        rtl_inp_ptr = ++temp_ptr;
    }
 
    {    
        UInt<4>* temp_ptr = (UInt<4>*)rtl_inp_ptr;
        dut->io_ins_3 = *temp_ptr;
    }
 
    dut->reset = UInt<1>(0);
    return;
}

void RTLEvent::control_sigs(VecShiftRegister* dut) {
    
    dut->reset = UInt<1>(1);
    void* rtl_ctrl_ptr = ctrl_ptr;
    {
        UInt<1>* temp_ptr = (UInt<1>*)rtl_ctrl_ptr;
        dut->io_load = *temp_ptr;
        rtl_ctrl_ptr = ++temp_ptr;
    }

    {
        UInt<1>* temp_ptr = (UInt<1>*)rtl_ctrl_ptr;
        dut->io_shift = *temp_ptr;
    }

    dut->reset = UInt<1>(0);
    return;
}

