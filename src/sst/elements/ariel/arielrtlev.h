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
#include <unordered_map>
#include <vector>
#include <string>

typedef std::vector<std::pair<std::string, unsigned int>> TYPEINFO;

using namespace SST;

namespace SST {
namespace ArielComponent {

typedef struct RtlSharedData {
    TYPEINFO* rtl_inp_info;
    TYPEINFO* rtl_ctrl_info;

    uint64_t cacheLineSize;
    
    void* rtl_inp_ptr;
    void* rtl_ctrl_ptr;   
    void* updated_rtl_params;

    std::unordered_map<uint64_t, uint64_t> *pageTable, *translationCache;
    std::deque<uint64_t>* freePages;
    uint64_t pageSize;
    uint32_t translationCacheEntries;
    bool translationEnabled;

 /*   uint64_t rtl_inp_VA;
    uint64_t rtl_ctrl_VA;
    uint64_t updated_rtl_params_VA;

    uint64_t rtl_inp_PA;
    uint64_t rtl_ctrl_PA;
    uint64_t updated_rtl_params_PA;*/

    std::unordered_map<>* PageTable;
    size_t rtl_inp_size;
    size_t rtl_ctrl_size;
    size_t updated_rtl_params_size;
    bool update_params;

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
      
/*      uint64_t get_rtl_inp_PA() {
          return RtlData->rtl_inp_PA;
      }

      uint64_t get_rtl_ctrl_PA() {
          return RtlData->rtl_ctrl_PA;
      }
      
      uint64_t get_updated_rtl_params_PA() {
          return RtlData->updated_rtl_params_PA;
      }*/

      size_t get_rtl_inp_size() {
          return RtlData->rtl_inp_size;
      }
 
      size_t get_rtl_ctrl_size() {
          return RtlData->rtl_ctrl_size;
      }
      
      size_t get_updated_rtl_params_size() {
          return RtlData->updated_rtl_params_size;
      }

      uint64_t get_cachelinesize() {
          return RtlData->cacheLineSize;
      }

      bool isUpdate_params() {
          return RtlData->update_params;
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

/*      void set_rtl_inp_PA(uint64_t setPA) {
          RtlData->rtl_inp_PA = setPA;
      }

      void set_rtl_ctrl_PA(uint64_t setPA) {
          RtlData->rtl_ctrl_PA = setPA;
      }
      
      void set_updated_rtl_params_PA(uint64_t setPA) {
          RtlData->updated_rtl_params_PA = setPA;*/
      }

      void set_rtl_inp_size(size_t size) {
          RtlData->rtl_inp_size = size;
      }
 
      void set_rtl_ctrl_size(size_t size) {
          RtlData->rtl_ctrl_size = size;
      }
      
      void set_updated_rtl_params_size(size_t size) {
          RtlData->updated_rtl_params_size = size;
      }

      void set_cachelinesize(uint64_t cachelinesize) {
          RtlData->cacheLineSize = cachelinesize;
      }

      void set_isUpdate_params(bool update) {
          RtlData->update_params = update;
      }

      void setEndSim(bool endIt) {
          endSim = endIt;
      }

      void setEventRecvAck(bool EvRecvd) {
          EvRecvAck = EvRecvd;
      }
 
      ImplementSerializable(SST::ArielComponent::ArielRtlEvent);
};

}
}

#endif
