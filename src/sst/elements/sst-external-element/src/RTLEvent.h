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

#ifndef _RTLEVENT_H
#define _RTLEVENT_H

#include "uint.h"
#include "VecShiftRegister_O1.h"
#include "mlm.h"

namespace SST {
namespace VecShiftReg {

class RTLEvent : public SST::Event
{
public:
    typedef std::vector<char> dataVec;
    bool update_registers, verbose, done_reset, updated, update_eval_args;
    RTLEvent() : SST::Event() { }
    dataVec payload;

    void serialize_order(SST::Core::Serialization::serializer &ser)  override {
        Event::serialize_order(ser);
        ser & payload;
    }

    ImplementSerializable(SST::VecShiftReg::RTLEvent);
    void input_sigs(VecShiftRegister*);
    void control_sigs(VecShiftRegister*);
    void *inp_ptr, *ctrl_ptr;
    TYPEINFO inp_info, ctrl_info;

    //Cast all the variables to 4 byte integer types for uniform storage for now. Later, we either will remove UInt and SInt and use native types. Even then we would need to cast the every variables based on type, width and order while storing in shmem and accordinly access it at runtime from shmem. Looks dificult task as of now.    
    /*void* input_ctrl_registers;
    UInt<4>* inp_ctrl_reg;*/
};

} // namespace VecShiftReg
} // namespace SST

#endif /* _RTLEVENT_H */
