// Copyright 2009-2015 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2015, Sandia Corporation
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#include <vector>
#include <utility>

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

using namespace std;
typedef std::vector<std::pair<std::string, unsigned int>> TYPEINFO; 
typedef int mlm_Tag;


/*void* mlm_malloc(size_t size, int level);
void  mlm_free(void* ptr);
mlm_Tag mlm_memcpy(void* dest, void* src, size_t length);
void mlm_waitComplete(mlm_Tag in);
void mlm_set_pool(int pool);*/

class RTL_shmem_info {

  //  public:
        /*RTL input information and control information corresponds to type(std::string) and size (in bytes) to which the void pointer will be casted to. Order of each stored input and control values should be maintained in accordance with the stored information in the TYPEINFO(std::array). Increment of void pointer once casted will align with the stored inp/ctrl data type. Any wrong cast can corrupt all the input and control signal information. */
    private:      
        TYPEINFO rtl_inp_info;
        TYPEINFO rtl_ctrl_info;

        void* rtl_inp_ptr;
        void* rtl_ctrl_ptr;
        void* updated_rtl_params;

        /*void calc_and_alloc();
        size_t size_of_shmem;    
        void ariel_assign_RTL_inputs();
        void ariel_assign_RTL_controls();*/

    public:

        RTL_shmem_info(size_t inp_size, size_t ctrl_size) {
            rtl_inp_ptr = malloc(inp_size); 
            rtl_ctrl_ptr = malloc(ctrl_size); 
            updated_rtl_params = malloc(7 * sizeof(bool) + sizeof(uint64_t)); 
        }
        ~RTL_shmem_info() {
            free(rtl_inp_ptr);
            free(rtl_ctrl_ptr);
            free(updated_rtl_params);
        }
        TYPEINFO get_inp_info() { return rtl_inp_info; }
        TYPEINFO get_ctrl_info() { return rtl_ctrl_info; } 
        void* get_inp_ptr() { return rtl_inp_ptr; }
        void* get_ctrl_ptr() { return rtl_ctrl_ptr; }
        void* get_updated_rtl_params() { return updated_rtl_params; }
        void set_inp_info(TYPEINFO& info) { rtl_inp_info = info; }
        void set_ctrl_info(TYPEINFO& info) { rtl_ctrl_info = info; }
}

void start_RTL_sim(RTL_shmem_info* shmem) {
   
    //Empty function as PIN tool's dynamic instrumentation will replace it with its Ariel Equivalent API in PIN frontend - fesimple.
    
    return; 
}

void update_RTL_signals() { 
    //Empty function as PIN tool's dynamic instrumentation will replace it with Ariel Equivalent API in PIN frontend - fesimple.
    return;
}

class Update_RTL_Params {
    
    private:
        bool update_inp, update_ctrl, update_eval_args, update_reg, verbose, done_reset, sim_done;
        uint64_t sim_cycles;
    public:

        Update_RTL_Params(bool inp = true, bool ctrl = true, bool eval_args = true, uint64_t cycles = 1, bool reg = false, bool verbose = true, bool done_rst = false, bool done = false) {
            update_inp = inp;
            update_ctrl = ctrl;
            update_eval_args = eval_args;
            update_reg = reg;
            verbose = verbose;
            done_reset = done_rst;
            sim_done = done;
            sim_cycles = cycles;
        }

        void storetomem(RTL_shmem_info* shmem) {
            bool* Ptr = (bool*)shmem->get_updated_rtl_params();
            *Ptr = update_inp;
            Ptr++;
            *Ptr = update_ctrl;
            Ptr++;
            *Ptr = update_eval_args;
            Ptr++;
            uint64_t* Cycles_ptr  = (uint64_t*)Ptr;
            *Cycles_ptr = sim_cycles;
            Cycles_ptr++;
            Ptr = (bool*)Cycles_ptr;
            *Ptr = sim_done;
            Ptr++;
            *Ptr = update_reg;
            Ptr++;
            *Ptr = verbose;
            Ptr++;
            *Ptr = done_reset;
            
            return;
        }
}

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif
