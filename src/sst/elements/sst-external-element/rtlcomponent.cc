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
#include <sst/core/event.h>
#include "rtlcomponent.h"
#include "rtlevent.h"
#include "rtl_header.h"
#include "rtlmemmgr.h"

using namespace SST;
using namespace std;
using namespace SST::RtlComponent;
using namespace SST::Interfaces;

//#define RTL_CORE_VERBOSE(LEVEL, OUTPUT) if(verbosity >= (LEVEL)) OUTPUT

Rtlmodel::Rtlmodel(SST::ComponentId_t id, SST::Params& params) :
	SST::Component(id)/*, verbosity(static_cast<uint32_t>(out->getVerboseLevel()))*/ {

    bool found;
    dut = new Rtlheader;
    axiport = new AXITop;
    ev = new RTLEvent();
    RtlAckEv = new ArielComponent::ArielRtlEvent();
	output.init("Rtlmodel-" + getName() + "-> ", 1, 0, SST::Output::STDOUT);

	RTLClk  = params.find<std::string>("ExecFreq", "a" , found);
    if (!found) {
        Simulation::getSimulation()->getSimulationOutput().fatal(CALL_INFO, -1,"couldn't find work per cycle\n");
    }

	maxCycles = params.find<Cycle_t>("maxCycles", 0, found);
    if (!found) {
        Simulation::getSimulation()->getSimulationOutput().fatal(CALL_INFO, -1,"couldn't find work per cycle\n");
    }

	/*if(RTLClk == NULL || RTLClk == "0") 
		output.fatal(CALL_INFO, -1, "Error: printFrequency must be greater than zero.\n");*/

 	output.verbose(CALL_INFO, 1, 0, "Config: maxCycles=%" PRIu64 ", RTL Clock Freq=%s\n",
		static_cast<uint64_t>(maxCycles), RTLClk.c_str());

	// Just register a plain clock for this simple example
    output.verbose(CALL_INFO, 1, 0, "Registering RTL clock at %s\n", RTLClk.c_str());
   
   //set our clock 
   timeConverter = registerClock(RTLClk, new Clock::Handler<Rtlmodel>(this, &Rtlmodel::clockTick));
   writePayloads = params.find<int>("writepayloadtrace") == 0 ? false : true;

    //Configure and register Event Handler for ArielRtllink
   ArielRtlLink = configureLink("ArielRtllink", new Event::Handler<Rtlmodel>(this, &Rtlmodel::handleArielEvent)); 

    // Find all the components loaded into the "memory" slot
    // Make sure all cores have a loaded subcomponent in their slot
    cacheLink = loadUserSubComponent<Interfaces::SimpleMem>("memory", ComponentInfo::SHARE_NONE, timeConverter, new SimpleMem::Handler<Rtlmodel>(this, &Rtlmodel::handleMemEvent));
    if(!cacheLink) {
       std::string interfaceName = params.find<std::string>("memoryinterface", "memHierarchy.memInterface");
       output.verbose(CALL_INFO, 1, 0, "Memory interface to be loaded is: %s\n", interfaceName.c_str());
       
       Params interfaceParams = params.find_prefix_params("memoryinterfaceparams.");
       interfaceParams.insert("port", "RtlCacheLink");
       cacheLink = loadAnonymousSubComponent<Interfaces::SimpleMem>(interfaceName, "memory", 0, ComponentInfo::SHARE_PORTS | ComponentInfo::INSERT_STATS,
               interfaceParams, timeConverter, new SimpleMem::Handler<Rtlmodel>(this, &Rtlmodel::handleMemEvent));
       
       if (!cacheLink)
           output.fatal(CALL_INFO, -1, "%s, Error loading memory interface\n", getName().c_str());
   }

    std::string memorymanager = params.find<std::string>("memmgr", "rtl.MemoryManagerSimple");
    if (NULL != (memmgr = loadUserSubComponent<RtlMemoryManager>("memmgr"))) {
        output.verbose(CALL_INFO, 1, 0, "Loaded memory manager: %s\n", memmgr->getName().c_str());
    } else {
        // Warn about memory levels and the selected memory manager if needed
        if (memorymanager == "rtl.MemoryManagerSimple" /*&& memLevels > 1*/) {
            output.verbose(CALL_INFO, 1, 0, "Warning - the default 'rtl.MemoryManagerSimple' does not support multiple memory levels. Configuring anyways but memorylevels will be 1.\n");
            params.insert("memmgr.memorylevels", "1", true);
        }

        output.verbose(CALL_INFO, 1, 0, "Loading memory manager: %s\n", memorymanager.c_str());
        Params mmParams = params.find_prefix_params("memmgr.");
        memmgr = loadAnonymousSubComponent<RtlMemoryManager>(memorymanager, "memmgr", 0, ComponentInfo::SHARE_NONE | ComponentInfo::INSERT_STATS, mmParams);
        if (NULL == memmgr) output.fatal(CALL_INFO, -1, "Failed to load memory manager: %s\n", memorymanager.c_str());
    }

    output.verbose(CALL_INFO, 1, 0, "RTL Memory manager construction is completed.\n");

   pendingTransactions = new std::unordered_map<SimpleMem::Request::id_t, SimpleMem::Request*>();
   pending_transaction_count = 0;
   isStalled = false;

    statReadRequests  = registerStatistic<uint64_t>( "read_requests");
    statWriteRequests = registerStatistic<uint64_t>( "write_requests");
    statReadRequestSizes = registerStatistic<uint64_t>( "read_request_sizes");
    statWriteRequestSizes = registerStatistic<uint64_t>( "write_request_sizes");
    statSplitReadRequests = registerStatistic<uint64_t>( "split_read_requests");
    statSplitWriteRequests = registerStatistic<uint64_t>( "split_write_requests");
    statFlushRequests = registerStatistic<uint64_t>( "flush_requests");
    statFenceRequests = registerStatistic<uint64_t>( "fence_requests");


   // Tell SST to wait until we authorize it to exit
   registerAsPrimaryComponent();
   primaryComponentDoNotEndSim();

   assert(ArielRtlLink);
}

Rtlmodel::~Rtlmodel() {
    delete ev;
    delete dut;
    delete axiport;
    delete RtlAckEv;
}

void Rtlmodel::setup() {
    dut->reset = UInt<1>(1);
	output.verbose(CALL_INFO, 1, 0, "Component is being setup.\n");
    
}

//Nothing to add in finish as of now. Need to see what could be added.
void Rtlmodel::finish() {
	output.verbose(CALL_INFO, 1, 0, "Component is being finished.\n");
}

//clockTick will actually execute the RTL design at every cycle based on the input and control signals updated by Ariel CPU or Event Handler.
bool Rtlmodel::clockTick( SST::Cycle_t currentCycle ) {

    /*uint64_t DUTclk; 
    char* endPtr;
    DUTclk = strtoull(RTLClk.c_str(), &endPtr, 10);
    DUTcycles = 10^9/DUTclk;
	unitAlgebra clock_ua(RTLClk);
    if( currentCycle %  == 0 ) {
		output.verbose(CALL_INFO, 1, 0, "Hello World!\n");
	}*/

    if(!isStalled) {
        dut->eval(update_registers, verbose, done_reset);
    }

    if((axi_tvalid_$old ^ axi_tvalid_$next) || (axi_tdata_$old ^ axi_tdata_$next))  {
        handleAXISignals();
        axiport->eval(true, false, false);
    }
	tickCount++;

    axi_tdata_$old = axi_tdata_$next;
    axi_tvalid_$old = axi_tvalid_$next;
    axi_tready_$old = axi_tready_$next;


	if( tickCount >= dynCycles || tickCount >= maxCycles ) {
        if(sim_done) {
            primaryComponentOKToEndSim();  //Tell the SST that it can finish the simulation.
            RtlAckEv->setEndSim(true);
            ArielRtlLink->send(RtlAckEv);
            return true;
        }
	} 
    
    else 
        return false;
}

/*Event Handle will be called by Ariel CPU once it(Ariel CPU) puts the input and control signals in the shared memory. Now, we need to modify Ariel CPU code for that.
Event handler will update the input and control signal based on the workload/C program to be executed.
Don't know what should be the argument of Event handle as of now. As, I think we don't need any argument. It's just a requst/call by Ariel CPU to update input and control signals.*/
void Rtlmodel::handleArielEvent(SST::Event *event) {
    /*
    * Event will pick information from shared memory. (What will be the use of Event queue.)
    * Need to insert code for it. 
    * Probably void pointers will be used to get the data from the shared memory which will get casted based on the width set by the user at runtime.
    * void pointers will be defined by Ariel CPU and passed as parameters through SST::Event to the C++ model. 
    * As of now, shared memory is like a scratch-pad or heap which is passive without any intelligent performance improving stuff like TLB, Cache hierarchy, accessing mechanisms(VIPT/PIPT) etc.  
    */
    ArielComponent::ArielRtlEvent* ariel_ev = dynamic_cast<ArielComponent::ArielRtlEvent*>(event);
    RtlAckEv->setEventRecvAck(true);
    ArielRtlLink->send(RtlAckEv);

    output.verbose(CALL_INFO, 1, 0, "\nVecshiftReg RTL Event handle called \n");

    memmgr->AssignRtlMemoryManagerSimple(ariel_ev->RtlData->pageTable, ariel_ev->RtlData->freePages, ariel_ev->RtlData->pageSize);
    memmgr->AssignRtlMemoryManagerCache(ariel_ev->RtlData->translationCache, ariel_ev->RtlData->translationCacheEntries, ariel_ev->RtlData->translationEnabled);

    //Update all the virtual address pointers in RTLEvent class
    ev->updated_rtl_params = ariel_ev->get_updated_rtl_params();
    ev->inp_ptr = ariel_ev->get_rtl_inp_ptr(); 
    ev->inp_info = ariel_ev->get_rtl_inp_info();
    ev->ctrl_ptr = ariel_ev->get_rtl_ctrl_ptr(); 
    ev->ctrl_info = ariel_ev->get_rtl_ctrl_info();
    cacheLineSize = ariel_ev->RtlData->cacheLineSize;

    //Creating Read Event from memHierarchy for the above virtual address pointers
    RtlReadEvent* rtlrev_params = new RtlReadEvent((uint64_t)ariel_ev->get_updated_rtl_params(),(uint32_t)ariel_ev->get_updated_rtl_params_size()); 
    RtlReadEvent* rtlrev_inp_ptr = new RtlReadEvent((uint64_t)ariel_ev->get_rtl_inp_ptr(),(uint32_t)ariel_ev->get_rtl_inp_size()); 
    RtlReadEvent* rtlrev_ctrl_ptr = new RtlReadEvent((uint64_t)ariel_ev->get_rtl_ctrl_ptr(),(uint32_t)ariel_ev->get_rtl_ctrl_size()); 
    RtlReadEvent* rtlrev_inp_info = new RtlReadEvent((uint64_t)ariel_ev->get_rtl_inp_info(),(uint32_t)sizeof(*(ariel_ev->get_rtl_inp_info()))); 
    RtlReadEvent* rtlrev_ctrl_info = new RtlReadEvent((uint64_t)ariel_ev->get_rtl_ctrl_info(),(uint32_t)sizeof(*(ariel_ev->get_rtl_ctrl_info()))); 

    size_t size = ariel_ev->get_updated_rtl_params_size() + ariel_ev->get_rtl_inp_size() + ariel_ev->get_rtl_ctrl_size() + (size_t)sizeof(ariel_ev->get_rtl_inp_info()) + (size_t)sizeof(ariel_ev->get_rtl_ctrl_info());
    uint8_t* data = (uint8_t*)malloc(size);
    setBaseDataAddress(data);

    //Initiating the read request from memHierarchy
    generateReadRequest(rtlrev_params);
    generateReadRequest(rtlrev_inp_ptr);
    generateReadRequest(rtlrev_ctrl_ptr);
    generateReadRequest(rtlrev_inp_info);
    generateReadRequest(rtlrev_ctrl_info);
}

void Rtlmodel::handleAXISignals() {
    axiport->readerFrontend.done = 0;
    axiport->readerFrontend.enable = 1;
    axiport->readerFrontend.length = 64;
    axiport->io_read_tdata = axi_tdata_$next; 
    axiport->io_read_tvalid = axi_tvalid_$next; 
    axiport->io_read_tready = axi_tready_$next; 
    axiport->cmd_queue.push('r');
}

/*void Rtlmodel::updateRtlsignals(uint64_t* address) {
   * switch(address) {
        case ev->udpated_rtl_params:
            bool* ptr = (bool*)address;
            ev->update_inp = *ptr;
            ev->update_ctrl = *++ptr;
            ev->update_eval_args = *++ptr; 
            break;
        case ev->inp_ptr:
            ev->
            
    }
}*/
    //output.verbose(CALL_INFO, 1, 0, "First value of RTL params is: %d", *updated_rtl_params);
    
/*    if(update_inp) {
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
        update_registers = *++updated_rtl_params;
        verbose = *++updated_rtl_params;
        done_reset = *++updated_rtl_params;
    }

    sim_done = *++updated_rtl_params;

    uint64_t* dynCycles_ptr = (uint64_t*)(++updated_rtl_params);
    dynCycles = *dynCycles_ptr;
 
    //loading initial values to the RTL design
    dut->eval(update_registers, verbose, done_reset);

}*/

void Rtlmodel::handleMemEvent(SimpleMem::Request* event) {
    output.verbose(CALL_INFO, 4, 0, " handling a memory event in RtlModel.\n");
    SimpleMem::Request::id_t mev_id = event->id;
    auto find_entry = pendingTransactions->find(mev_id);
    if(find_entry != pendingTransactions->end()) {
        output.verbose(CALL_INFO, 4, 0, "Correctly identified event in pending transactions, removing from list, before there are: %" PRIu32 " transactions pending.\n", (uint32_t) pendingTransactions->size());
       
        //int index = event->getVirtualAddress() - getBaseDataAddress();
        int index = getDataAddress() - getBaseDataAddress();
        int i;
        //Actual reading of data from memEvent and storing it to getDataAddress
        for(i = 0; i < event->data.size(); i++)
            getDataAddress()[index+i] = event->data[i]; 

        setDataAddress(getDataAddress()+index+i);
        if(event->getVirtualAddress() == (uint64_t)ev->updated_rtl_params) {
            bool* ptr = (bool*)getBaseDataAddress();
            output.verbose(CALL_INFO, 1, 0, "Updated Rtl Params is: %d\n",*ptr);
        }
        //UpdateRtlParams(dataAddress);

        pendingTransactions->erase(find_entry);
        pending_transaction_count--;

        if(isStalled && pending_transaction_count == 0)
            isStalled = false;
    } 
    
    else 
        output.fatal(CALL_INFO, -4, "Memory event response to VecShiftReg was not found in pending list.\n");
        
    delete event;
}

void Rtlmodel::commitReadEvent(const uint64_t address,
            const uint64_t virtAddress, const uint32_t length) {
    if(length > 0) {
        SimpleMem::Request *req = new SimpleMem::Request(SimpleMem::Request::Read, address, length);
        req->setVirtualAddress(virtAddress);
        
        pending_transaction_count++;
        pendingTransactions->insert(std::pair<SimpleMem::Request::id_t, SimpleMem::Request*>(req->id, req));

        /*if(enableTracing) {
                printTraceEntry(true, (const uint64_t) req->addrs[0], (const uint32_t) length);
        }*/

        // Actually send the event to the cache
        cacheLink->sendRequest(req);
    }
}

void Rtlmodel::commitWriteEvent(const uint64_t address,
        const uint64_t virtAddress, const uint32_t length, const uint8_t* payload) {

    if(length > 0) {
        SimpleMem::Request *req = new SimpleMem::Request(SimpleMem::Request::Write, address, length);
        req->setVirtualAddress(virtAddress);

        if( writePayloads ) {
            //if(verbosity >= 16) {
                char* buffer = new char[64];
                std::string payloadString = "";
                for(int i = 0; i < length; ++i) {
                    sprintf(buffer, "0x%X ", payload[i]);
                    payloadString.append(buffer);
                }

                delete[] buffer;

                output.verbose(CALL_INFO, 16, 0, "Write-Payload: Len=%" PRIu32 ", Data={ %s } %p\n",
                        length, payloadString.c_str(), (void*)virtAddress);
            //}
            req->setPayload( (uint8_t*) payload, length );
        }
        
        pending_transaction_count++;
        pendingTransactions->insert( std::pair<SimpleMem::Request::id_t, SimpleMem::Request*>(req->id, req) );
        
        /*if(enableTracing) {
            printTraceEntry(false, (const uint64_t) req->addrs[0], (const uint32_t) length);
        }*/

        // Actually send the event to the cache
        cacheLink->sendRequest(req);
    }
}

void Rtlmodel::generateReadRequest(RtlReadEvent* rEv) {

    const uint64_t readAddress = rEv->getAddress();
    const uint64_t readLength  = std::min((uint64_t) rEv->getLength(), cacheLineSize); // Trim to cacheline size (occurs rarely for instructions such as xsave and fxsave)

    // NOTE: Physical and virtual addresses may not be aligned the same w.r.t. line size if map-on-malloc is being used (arielinterceptcalls != 0), so use physical offsets to determine line splits
    // There is a chance that the non-alignment causes an undetected bug if an access spans multiple malloc regions that are contiguous in VA space but non-contiguous in PA space.
    // However, a single access spanning multiple malloc'd regions shouldn't happen...
    // Addresses mapped via first touch are always line/page aligned
    const uint64_t physAddr = memmgr->translateAddress(readAddress);
    const uint64_t addr_offset  = physAddr % ((uint64_t) cacheLineSize);

    if((addr_offset + readLength) <= cacheLineSize) {
        output.verbose(CALL_INFO, 4, 0, " generating a non-split read request: Addr=%" PRIu64 " Length=%" PRIu64 "\n",
                            readAddress, readLength);

        // We do not need to perform a split operation

        output.verbose(CALL_INFO, 4, 0, " issuing read, VAddr=%" PRIu64 ", Size=%" PRIu64 ", PhysAddr=%" PRIu64 "\n",
                            readAddress, readLength, physAddr);

        commitReadEvent(physAddr, readAddress, (uint32_t) readLength);
    } else {
        output.verbose(CALL_INFO, 4, 0, " generating a split read request: Addr=%" PRIu64 " Length=%" PRIu64 "\n",
                            readAddress, readLength);

        // We need to perform a split operation
        const uint64_t leftAddr = readAddress;
        const uint64_t leftSize = cacheLineSize - addr_offset;

        const uint64_t rightAddr = (readAddress + ((uint64_t) cacheLineSize)) - addr_offset;
        const uint64_t rightSize = readLength - leftSize;

        const uint64_t physLeftAddr = physAddr;
        const uint64_t physRightAddr = memmgr->translateAddress(rightAddr);

        output.verbose(CALL_INFO, 4, 0, " issuing split-address read, LeftVAddr=%" PRIu64 ", RightVAddr=%" PRIu64 ", LeftSize=%" PRIu64 ", RightSize=%" PRIu64 ", LeftPhysAddr=%" PRIu64 ", RightPhysAddr=%" PRIu64 "\n",
                            leftAddr, rightAddr, leftSize, rightSize, physLeftAddr, physRightAddr);

        if(((physLeftAddr + leftSize) % cacheLineSize) != 0) {
            output.fatal(CALL_INFO, -4, "Error leftAddr=%" PRIu64 " + size=%" PRIu64 " is not a multiple of cache line size: %" PRIu64 "\n",
                    leftAddr, leftSize, cacheLineSize);
        }

        commitReadEvent(physLeftAddr, leftAddr, (uint32_t) leftSize);
        commitReadEvent(physRightAddr, rightAddr, (uint32_t) rightSize);

        statSplitReadRequests->addData(1);
    }

    statReadRequests->addData(1);
    statReadRequestSizes->addData(readLength);
    delete rEv;
}

void Rtlmodel::generateWriteRequest(RtlWriteEvent* wEv) {

    const uint64_t writeAddress = wEv->getAddress();
    fprintf(stderr, "\n Handle Write request generated for VA: %" PRIu64, writeAddress);
    const uint64_t writeLength  = std::min((uint64_t) wEv->getLength(), cacheLineSize); // Trim to cacheline size (occurs rarely for instructions such as xsave and fxsave)

    // See note in handleReadRequest() on alignment issues
    const uint64_t physAddr = memmgr->translateAddress(writeAddress);
    fprintf(stderr, "\n Handle Write request generated for PA: %" PRIu64, physAddr);
    const uint64_t addr_offset  = physAddr % ((uint64_t) cacheLineSize);

    // We do not need to perform a split operation
    if((addr_offset + writeLength) <= cacheLineSize) {
        
        output.verbose(CALL_INFO, 4, 0, " generating a non-split write request: Addr=%" PRIu64 " Length=%" PRIu64 "\n",
                            writeAddress, writeLength);


        output.verbose(CALL_INFO, 4, 0, " issuing write, VAddr=%" PRIu64 ", Size=%" PRIu64 ", PhysAddr=%" PRIu64 "\n",
                            writeAddress, writeLength, physAddr);

        if(writePayloads) {
            uint8_t* payloadPtr = wEv->getPayload();
            commitWriteEvent(physAddr, writeAddress, (uint32_t) writeLength, payloadPtr);
        } else {
            commitWriteEvent(physAddr, writeAddress, (uint32_t) writeLength, NULL);
        }
    } else {
        output.verbose(CALL_INFO, 4, 0, " generating a split write request: Addr=%" PRIu64 " Length=%" PRIu64 "\n",
                            writeAddress, writeLength);

        // We need to perform a split operation
        const uint64_t leftAddr = writeAddress;
        const uint64_t leftSize = cacheLineSize - addr_offset;

        const uint64_t rightAddr = (writeAddress + ((uint64_t) cacheLineSize)) - addr_offset;
        const uint64_t rightSize = writeLength - leftSize;

        const uint64_t physLeftAddr = physAddr;
        const uint64_t physRightAddr = memmgr->translateAddress(rightAddr);

        output.verbose(CALL_INFO, 4, 0, " issuing split-address write, LeftVAddr=%" PRIu64 ", RightVAddr=%" PRIu64 ", LeftSize=%" PRIu64 ", RightSize=%" PRIu64 ", LeftPhysAddr=%" PRIu64 ", RightPhysAddr=%" PRIu64 "\n",
                            leftAddr, rightAddr, leftSize, rightSize, physLeftAddr, physRightAddr);

        if(((physLeftAddr + leftSize) % cacheLineSize) != 0) {
            output.fatal(CALL_INFO, -4, "Error leftAddr=%" PRIu64 " + size=%" PRIu64 " is not a multiple of cache line size: %" PRIu64 "\n",
                            leftAddr, leftSize, cacheLineSize);
        }

        if(writePayloads) {
            uint8_t* payloadPtr = wEv->getPayload();
            commitWriteEvent(physLeftAddr, leftAddr, (uint32_t) leftSize, payloadPtr);
            commitWriteEvent(physRightAddr, rightAddr, (uint32_t) rightSize, &payloadPtr[leftSize]);
        } else {
            commitWriteEvent(physLeftAddr, leftAddr, (uint32_t) leftSize, NULL);
            commitWriteEvent(physRightAddr, rightAddr, (uint32_t) rightSize, NULL);
        }
        statSplitWriteRequests->addData(1);
    }

    statWriteRequests->addData(1);
    statWriteRequestSizes->addData(writeLength);
    delete wEv;
}

uint8_t* Rtlmodel::getDataAddress() {
    return dataAddress;
}

void Rtlmodel::setDataAddress(uint8_t* virtAddress){
    dataAddress = virtAddress;
}

uint8_t* Rtlmodel::getBaseDataAddress(){
    return baseDataAddress;
}

void Rtlmodel::setBaseDataAddress(uint8_t* virtAddress){
    baseDataAddress = virtAddress;
}
