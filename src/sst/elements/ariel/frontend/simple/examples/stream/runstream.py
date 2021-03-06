import sst
import os

sst.setProgramOption("timebase", "1ps")
sst_workdir = os.getenv( "SST_WORKDIR" )
#app = sst_root + "/sst-elements/src/sst/elements/ariel/frontend/simple/examples/stream/stream"
app = sst_workdir+"/sst-tools/tools/ariel/femlm/examples/stream/mlmstream" 

if not os.path.exists(app):
    app = os.getenv( "OMP_EXE" )
    printf("OS PATH DOESN'T EXIST")
ariel = sst.Component("Shubham", "ariel.ariel")
ariel.addParams({
        "verbose" : "1",
        "maxcorequeue" : "256",
        "maxissuepercycle" : "2",
        "pipetimeout" : "0",
        "executable" : app,
        "arielmode" : "1",
        #"arielinterceptcalls" : "1",
        "launchparamcount" : 1,
        #"arielstack" : "1",
        #"mallocmapfile" : "memory_map.log",
        "launchparam0" : "-ifeellucky",
        })

memmgr = ariel.setSubComponent("memmgr", "ariel.MemoryManagerSimple")

corecount = 1;

l1cache = sst.Component("l1cache", "memHierarchy.Cache")
l1cache.addParams({
        "cache_frequency" : "2 Ghz",
        "cache_size" : "64 KB",
        "coherence_protocol" : "MSI",
        "replacement_policy" : "lru",
        "associativity" : "8",
        "access_latency_cycles" : "1",
        "cache_line_size" : "64",
        "L1" : "1",
        "debug" : "0",
})

memctrl = sst.Component("memory", "memHierarchy.MemController")
memctrl.addParams({
        "clock" : "1GHz",
})

memory = memctrl.setSubComponent("backend", "memHierarchy.simpleMem")
memory.addParams({
        "access_time" : "10ns",
        "mem_size" : "2048MiB",
})

vecshiftregister = sst.Component("vecshiftregister", "vecshiftreg.vecShiftReg")
vecshiftregister.addParams({
        "ExecFreq" : "1 GHz",
        "maxCycles" : "100"
	})

cpu_cache_link = sst.Link("cpu_cache_link")
cpu_cache_link.connect( (ariel, "cache_link_0", "50ps"), (l1cache, "high_network_0", "50ps") )

memory_link = sst.Link("mem_bus_link")
memory_link.connect( (l1cache, "low_network_0", "50ps"), (memctrl, "direct_link", "50ps") )

cpu_rtl_link = sst.Link("cpu_rtl_link")
cpu_rtl_link.connect( (ariel, "rtl_link_0", "50ps"), (vecshiftregister, "Rtllink", "50ps") )

# Set the Statistic Load Level; Statistics with Enable Levels (set in
# elementInfoStatistic) lower or equal to the load can be enabled (default = 0)
sst.setStatisticLoadLevel(16)

# Set the desired Statistic Output (sst.statOutputConsole is default)
sst.setStatisticOutput("sst.statOutputConsole")
#sst.setStatisticOutput("sst.statOutputTXT", {"filepath" : "./TestOutput.txt"
#                                            })
#sst.setStatisticOutput("sst.statOutputCSV", {"filepath" : "./TestOutput.csv",
#                                                         "separator" : ", "
#                                            })

# Enable Individual Statistics for the Component with output at end of sim
# Statistic defaults to Accumulator
ariel.enableStatistics([
      "cycles",
      "active_cycles",
      "instruction_count",
      "read_requests",
      "write_requests"
])

l1cache.enableStatistics([
      #"CacheHits",
      "latency_GetS_hit",
      "latency_GetX_hit",
      "latency_GetS_miss",
      "latency_GetX_miss",
      "GetSHit_Arrival",
      "GetSHit_Blocked",
      "CacheMisses"
])
