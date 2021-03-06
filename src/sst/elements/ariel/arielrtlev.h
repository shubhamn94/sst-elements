// Copyright 2009-2020 Sandia Corporation. Under the terms
// of Contract DE-NA0003525 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2020, Sandia Corporation
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.


#ifndef _H_SST_ARIEL_RTL_EVENT
#define _H_SST_ARIEL_RTL_EVENT

#include "arielevent.h"
#include <vector>
#include <string>

typedef std::vector<std::pair<std::string, unsigned int>> TYPEINFO;

using namespace SST;

namespace SST {
namespace ArielComponent {

typedef struct RtlSharedData {
    TYPEINFO* rtl_inp_info;
    TYPEINFO* rtl_ctrl_info;
    
    void* rtl_inp_ptr;
    void* rtl_ctrl_ptr;   
    void* updated_rtl_params;
}RtlSharedData;

class ArielRtlEvent : public ArielEvent, public SST::Event {
   private:
      RtlSharedData* RtlData;
      bool endSim;
      bool EvRecvAck;

   public:
      ArielRtlEvent() : Event() {
        RtlData = new RtlSharedData;
        endSim = false;
        EvRecvAck = false;
      }
      ~ArielRtlEvent() { delete RtlData; }
      
      typedef std::vector<char> dataVec;
      dataVec payload;
      
      void serialize_order(SST::Core::Serialization::serializer &ser)  override {
          Event::serialize_order(ser);
          ser & payload;
      }
      
      ArielEventType getEventType() const {
         return RTL;
      }

      void* get_rtl_inp_ptr() {
          return RtlData->rtl_inp_ptr;
      }

      void* get_rtl_ctrl_ptr() {
          return RtlData->rtl_ctrl_ptr;
      }

      TYPEINFO* get_rtl_inp_info() {
          return RtlData->rtl_inp_info;
      }

      TYPEINFO* get_rtl_ctrl_info() {
          return RtlData->rtl_ctrl_info;
      }
      void* get_updated_rtl_params() {
          return RtlData->updated_rtl_params;
      }

      bool getEndSim() {
          return endSim;
      }

      bool getEventRecvAck() {
          return EvRecvAck;
      }

      void set_rtl_inp_info(TYPEINFO* info) {
          RtlData->rtl_inp_info = info;
      }

      void set_rtl_ctrl_info(TYPEINFO* info) {
          RtlData->rtl_ctrl_info = info;
      }
     
      void set_rtl_inp_ptr(void* setPtr) {
          RtlData->rtl_inp_ptr = setPtr;
      }

      void set_rtl_ctrl_ptr(void* setPtr) {
          RtlData->rtl_ctrl_ptr = setPtr;
      }
      
      void set_updated_rtl_params(void* setPtr) {
          RtlData->updated_rtl_params = setPtr;
      }

      void setEndSim(bool endIt) {
          endSim = endIt;
      }

      void setEventRecvAck(bool EvRecvd) {
          EvRecvAck = EvRecvd;
      }
 
      /*unsigned getFatCubinHandle() {
         return ca.register_function.fat_cubin_handle;
      }

      char* getDeviceFun() {
         return ca.register_function.device_fun;
      }

      uint64_t getHostFun() {
         return ca.register_function.host_fun;
      }

      void** getDevPtr() {
         return ca.cuda_malloc.dev_ptr;
      }

      size_t getSize() {
         printf("%d arielrtlevent\n", ca.cuda_malloc.size);
         return ca.cuda_malloc.size;
      }

      uint64_t get_src() {
         return ca.cuda_memcpy.src;
      }

      uint64_t get_dst() {
         return ca.cuda_memcpy.dst;
      }
      size_t get_count() {
         return ca.cuda_memcpy.count;
      }

      cudaMemcpyKind get_kind() {
         return ca.cuda_memcpy.kind;
      }

      uint8_t* get_data() {
         return ca.cuda_memcpy.data;
      }

      unsigned int get_gridDimx() {
         return ca.cfg_call.gdx;
      }

      unsigned int get_gridDimy() {
         return ca.cfg_call.gdy;
      }

      unsigned int get_gridDimz() {
         return ca.cfg_call.gdz;
      }

      unsigned int get_blockDimx() {
         return ca.cfg_call.bdx;
      }

      unsigned int get_blockDimy() {
         return ca.cfg_call.bdy;
      }

      unsigned int get_blockDimz() {
         return ca.cfg_call.bdz;
      }

      size_t get_shmem() {
         return ca.cfg_call.sharedMem;
      }

      cudaStream_t get_stream() {
         return ca.cfg_call.stream;
      }

      uint64_t get_address() {
         return ca.set_arg.address;
      }

      uint8_t* get_value() {
         return ca.set_arg.value;
      }

      size_t get_size() {
         return ca.set_arg.size;
      }

      size_t get_offset() {
         return ca.set_arg.offset;
      }

      uint64_t get_func(){
         return ca.cuda_launch.func;
      }

      uint64_t get_free_addr(){
         return ca.free_address;
      }

      unsigned getVarFatCubinHandle() {
         return ca.register_var.fatCubinHandle;
      }

      uint64_t getVarHostVar() {
         return ca.register_var.hostVar;
      }

      char* getVarDeviceName() {
         return ca.register_var.deviceName;
      }

      int getVarExt() {
         return ca.register_var.ext;
      }

      int getVarSize() {
         return ca.register_var.size;
      }

      int getVarConstant() {
         return ca.register_var.constant;
      }

      int getVarGlobal() {
         return ca.register_var.global;
      }

      uint64_t getMaxBlockHostFunc() {
         return ca.max_active_block.hostFunc;
      }

      uint64_t getMaxBlockBlockSize() {
         return ca.max_active_block.blockSize;
      }

      uint64_t getMaxBlockSMemSize() {
         return ca.max_active_block.dynamicSMemSize;
      }

      uint64_t getMaxBlockFlag() {
         return ca.max_active_block.flags;
      }*/

      ImplementSerializable(SST::ArielComponent::ArielRtlEvent);
};

}
}

#endif
