#
# OMNeT++/OMNEST Makefile for ccnSim
#
# This file was generated with the command:
#  opp_makemake --deep -f -X ./patch/ -X scripts/ -X networks/ -X modules/ -o ccnSim -X results/ -X ini/ -X manual/ -X doc/ -X file_routing/ -X ccn14distrib/ -X ccn14scripts/
#

# Name of target to be created (-o option)
TARGET = ccnSim$(EXE_SUFFIX)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(TKENV_LIBS) $(QTENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(TKENV_LIBS)
#USERIF_LIBS = $(QTENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I. \
    -Iinclude \
    -Iinclude/cost_related_decision_policies \
    -IinfoSim \
    -Ilogs \
    -IModelGraft-Sensitivity-Scripts \
    -Ipackets \
    -Isrc \
    -Isrc/clients \
    -Isrc/content \
    -Isrc/node \
    -Isrc/node/cache \
    -Isrc/node/strategy \
    -Isrc/statistics \
    -ITc_Values \
    -ITc_Values/Tc_Sensitivity \
    -ITc_Values/Tc_Sensitivity/LCE \
    -ITc_Values/Tc_Sensitivity/LCP \
    -ITc_Values/Tc_Sensitivity/TWO_TTL

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS =

# Output directory
PROJECT_OUTPUT_DIR = out
PROJECTRELATIVE_PATH =
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc, .msg and .sm files
OBJS = \
    $O/src/error_handling.o \
    $O/src/clients/client.o \
    $O/src/clients/client_IRM.o \
    $O/src/clients/client_ShotNoise.o \
    $O/src/clients/client_Window.o \
    $O/src/content/content_distribution.o \
    $O/src/content/ShotNoiseContentDistribution.o \
    $O/src/content/WeightedContentDistribution.o \
    $O/src/content/zipf.o \
    $O/src/content/zipf_sampled.o \
    $O/src/node/core_layer.o \
    $O/src/node/cache/base_cache.o \
    $O/src/node/cache/fifo_cache.o \
    $O/src/node/cache/lru_cache.o \
    $O/src/node/cache/random_cache.o \
    $O/src/node/cache/ttl_cache.o \
    $O/src/node/cache/ttl_name_cache.o \
    $O/src/node/cache/two_cache.o \
    $O/src/node/strategy/MonopathStrategyLayer.o \
    $O/src/node/strategy/MultipathStrategyLayer.o \
    $O/src/node/strategy/nrr.o \
    $O/src/node/strategy/nrr1.o \
    $O/src/node/strategy/parallel_repository.o \
    $O/src/node/strategy/ProbabilisticSplitStrategy.o \
    $O/src/node/strategy/random_repository.o \
    $O/src/node/strategy/spr.o \
    $O/src/node/strategy/strategy_layer.o \
    $O/src/statistics/statistics.o \
    $O/src/statistics/Tc_Solver.o \
    $O/packets/ccn_data_m.o \
    $O/packets/ccn_interest_m.o

# Message files
MSGFILES = \
    packets/ccn_data.msg \
    packets/ccn_interest.msg

# SM files
SMFILES =

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIB_SUBDIR = $(OMNETPP_LIB_DIR)/$(TOOLCHAIN_NAME)
OMNETPP_LIBS = -L"$(OMNETPP_LIB_SUBDIR)" -L"$(OMNETPP_LIB_DIR)" -loppmain$D $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)

COPTS = $(CFLAGS)  $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)
SMCOPTS =

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ($(MAKECMDGOALS),depend)
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# inserted from file 'makefrag':

# <<<
#------------------------------------------------------------------------------

# Main target
all: $O/$(TARGET)
	$(Q)$(LN) $O/$(TARGET) .

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	@echo Creating executable: $@
	$(Q)$(CXX) $(LDFLAGS) -o $O/$(TARGET)  $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY: all clean cleanall depend msgheaders smheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc $(MSGCOPTS) $?

%_sm.cc %_sm.h: %.sm
	$(qecho) SMC: $<
	$(Q)$(SMC) -c++ -suffix cc $(SMCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

smheaders: $(SMFILES:.sm=_sm.h)

clean:
	$(qecho) Cleaning...
	$(Q)-rm -rf $O
	$(Q)-rm -f ccnSim ccnSim.exe libccnSim.so libccnSim.a libccnSim.dll libccnSim.dylib
	$(Q)-rm -f ./*_m.cc ./*_m.h ./*_sm.cc ./*_sm.h
	$(Q)-rm -f include/*_m.cc include/*_m.h include/*_sm.cc include/*_sm.h
	$(Q)-rm -f include/cost_related_decision_policies/*_m.cc include/cost_related_decision_policies/*_m.h include/cost_related_decision_policies/*_sm.cc include/cost_related_decision_policies/*_sm.h
	$(Q)-rm -f infoSim/*_m.cc infoSim/*_m.h infoSim/*_sm.cc infoSim/*_sm.h
	$(Q)-rm -f logs/*_m.cc logs/*_m.h logs/*_sm.cc logs/*_sm.h
	$(Q)-rm -f ModelGraft-Sensitivity-Scripts/*_m.cc ModelGraft-Sensitivity-Scripts/*_m.h ModelGraft-Sensitivity-Scripts/*_sm.cc ModelGraft-Sensitivity-Scripts/*_sm.h
	$(Q)-rm -f packets/*_m.cc packets/*_m.h packets/*_sm.cc packets/*_sm.h
	$(Q)-rm -f src/*_m.cc src/*_m.h src/*_sm.cc src/*_sm.h
	$(Q)-rm -f src/clients/*_m.cc src/clients/*_m.h src/clients/*_sm.cc src/clients/*_sm.h
	$(Q)-rm -f src/content/*_m.cc src/content/*_m.h src/content/*_sm.cc src/content/*_sm.h
	$(Q)-rm -f src/node/*_m.cc src/node/*_m.h src/node/*_sm.cc src/node/*_sm.h
	$(Q)-rm -f src/node/cache/*_m.cc src/node/cache/*_m.h src/node/cache/*_sm.cc src/node/cache/*_sm.h
	$(Q)-rm -f src/node/strategy/*_m.cc src/node/strategy/*_m.h src/node/strategy/*_sm.cc src/node/strategy/*_sm.h
	$(Q)-rm -f src/statistics/*_m.cc src/statistics/*_m.h src/statistics/*_sm.cc src/statistics/*_sm.h
	$(Q)-rm -f Tc_Values/*_m.cc Tc_Values/*_m.h Tc_Values/*_sm.cc Tc_Values/*_sm.h
	$(Q)-rm -f Tc_Values/Tc_Sensitivity/*_m.cc Tc_Values/Tc_Sensitivity/*_m.h Tc_Values/Tc_Sensitivity/*_sm.cc Tc_Values/Tc_Sensitivity/*_sm.h
	$(Q)-rm -f Tc_Values/Tc_Sensitivity/LCE/*_m.cc Tc_Values/Tc_Sensitivity/LCE/*_m.h Tc_Values/Tc_Sensitivity/LCE/*_sm.cc Tc_Values/Tc_Sensitivity/LCE/*_sm.h
	$(Q)-rm -f Tc_Values/Tc_Sensitivity/LCP/*_m.cc Tc_Values/Tc_Sensitivity/LCP/*_m.h Tc_Values/Tc_Sensitivity/LCP/*_sm.cc Tc_Values/Tc_Sensitivity/LCP/*_sm.h
	$(Q)-rm -f Tc_Values/Tc_Sensitivity/TWO_TTL/*_m.cc Tc_Values/Tc_Sensitivity/TWO_TTL/*_m.h Tc_Values/Tc_Sensitivity/TWO_TTL/*_sm.cc Tc_Values/Tc_Sensitivity/TWO_TTL/*_sm.h

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES) $(SM_CC_FILES)  ./*.cc include/*.cc include/cost_related_decision_policies/*.cc infoSim/*.cc logs/*.cc ModelGraft-Sensitivity-Scripts/*.cc packets/*.cc src/*.cc src/clients/*.cc src/content/*.cc src/node/*.cc src/node/cache/*.cc src/node/strategy/*.cc src/statistics/*.cc Tc_Values/*.cc Tc_Values/Tc_Sensitivity/*.cc Tc_Values/Tc_Sensitivity/LCE/*.cc Tc_Values/Tc_Sensitivity/LCP/*.cc Tc_Values/Tc_Sensitivity/TWO_TTL/*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/src/error_handling.o: src/error_handling.cc \
  include/error_handling.h
$O/src/clients/client.o: src/clients/client.cc \
  include/ccn_data.h \
  include/ccn_interest.h \
  include/ccnsim.h \
  include/client.h \
  include/content_distribution.h \
  include/error_handling.h \
  include/statistics.h \
  include/zipf.h \
  include/zipf_sampled.h \
  packets/ccn_data_m.h \
  packets/ccn_interest_m.h
$O/src/clients/client_IRM.o: src/clients/client_IRM.cc \
  include/ccn_data.h \
  include/ccn_interest.h \
  include/ccnsim.h \
  include/client.h \
  include/client_IRM.h \
  include/content_distribution.h \
  include/error_handling.h \
  include/statistics.h \
  include/zipf.h \
  include/zipf_sampled.h \
  packets/ccn_data_m.h \
  packets/ccn_interest_m.h
$O/src/clients/client_ShotNoise.o: src/clients/client_ShotNoise.cc \
  include/ShotNoiseContentDistribution.h \
  include/ccn_data.h \
  include/ccn_interest.h \
  include/ccnsim.h \
  include/client.h \
  include/client_ShotNoise.h \
  include/content_distribution.h \
  include/error_handling.h \
  include/statistics.h \
  include/zipf.h \
  include/zipf_sampled.h \
  packets/ccn_data_m.h \
  packets/ccn_interest_m.h
$O/src/clients/client_Window.o: src/clients/client_Window.cc \
  include/ccn_data.h \
  include/ccn_interest.h \
  include/ccnsim.h \
  include/client.h \
  include/client_Window.h \
  include/content_distribution.h \
  include/error_handling.h \
  include/statistics.h \
  include/zipf.h \
  include/zipf_sampled.h \
  packets/ccn_data_m.h \
  packets/ccn_interest_m.h
$O/src/content/ShotNoiseContentDistribution.o: src/content/ShotNoiseContentDistribution.cc \
  include/ShotNoiseContentDistribution.h \
  include/ccnsim.h \
  include/client.h \
  include/content_distribution.h \
  include/core_layer.h \
  include/error_handling.h \
  include/zipf.h \
  include/zipf_sampled.h
$O/src/content/WeightedContentDistribution.o: src/content/WeightedContentDistribution.cc \
  include/WeightedContentDistribution.h \
  include/ccnsim.h \
  include/client.h \
  include/content_distribution.h \
  include/core_layer.h \
  include/error_handling.h \
  include/zipf.h \
  include/zipf_sampled.h
$O/src/content/content_distribution.o: src/content/content_distribution.cc \
  include/ShotNoiseContentDistribution.h \
  include/ccnsim.h \
  include/client.h \
  include/content_distribution.h \
  include/error_handling.h \
  include/zipf.h \
  include/zipf_sampled.h
$O/src/content/zipf.o: src/content/zipf.cc \
  include/zipf.h
$O/src/content/zipf_sampled.o: src/content/zipf_sampled.cc \
  include/ccnsim.h \
  include/client.h \
  include/error_handling.h \
  include/statistics.h \
  include/zipf_sampled.h
$O/src/node/core_layer.o: src/node/core_layer.cc \
  include/base_cache.h \
  include/ccn_data.h \
  include/ccn_interest.h \
  include/ccnsim.h \
  include/client.h \
  include/content_distribution.h \
  include/core_layer.h \
  include/decision_policy.h \
  include/error_handling.h \
  include/lru_cache.h \
  include/statistics.h \
  include/strategy_layer.h \
  include/ttl_name_cache.h \
  include/two_lru_policy.h \
  include/two_ttl_policy.h \
  include/zipf.h \
  include/zipf_sampled.h \
  packets/ccn_data_m.h \
  packets/ccn_interest_m.h
$O/src/node/cache/base_cache.o: src/node/cache/base_cache.cc \
  include/WeightedContentDistribution.h \
  include/always_policy.h \
  include/base_cache.h \
  include/betweenness_centrality.h \
  include/ccn_data.h \
  include/ccnsim.h \
  include/client.h \
  include/content_distribution.h \
  include/core_layer.h \
  include/cost_related_decision_policies/costaware_ancestor_policy.h \
  include/cost_related_decision_policies/costaware_parent_policy.h \
  include/cost_related_decision_policies/costaware_policy.h \
  include/cost_related_decision_policies/ideal_blind_policy.h \
  include/cost_related_decision_policies/ideal_costaware_grandparent_policy.h \
  include/cost_related_decision_policies/ideal_costaware_parent_policy.h \
  include/cost_related_decision_policies/ideal_costaware_policy.h \
  include/decision_policy.h \
  include/error_handling.h \
  include/fix_policy.h \
  include/lcd_policy.h \
  include/lru_cache.h \
  include/never_policy.h \
  include/prob_cache.h \
  include/statistics.h \
  include/ttl_name_cache.h \
  include/two_lru_policy.h \
  include/two_ttl_policy.h \
  include/zipf.h \
  include/zipf_sampled.h \
  packets/ccn_data_m.h
$O/src/node/cache/fifo_cache.o: src/node/cache/fifo_cache.cc \
  include/base_cache.h \
  include/ccnsim.h \
  include/client.h \
  include/error_handling.h \
  include/fifo_cache.h
$O/src/node/cache/lru_cache.o: src/node/cache/lru_cache.cc \
  include/base_cache.h \
  include/ccn_data.h \
  include/ccnsim.h \
  include/client.h \
  include/content_distribution.h \
  include/decision_policy.h \
  include/error_handling.h \
  include/lru_cache.h \
  include/two_lru_policy.h \
  include/zipf.h \
  include/zipf_sampled.h \
  packets/ccn_data_m.h
$O/src/node/cache/random_cache.o: src/node/cache/random_cache.cc \
  include/base_cache.h \
  include/ccnsim.h \
  include/client.h \
  include/random_cache.h
$O/src/node/cache/ttl_cache.o: src/node/cache/ttl_cache.cc \
  include/base_cache.h \
  include/ccn_data.h \
  include/ccnsim.h \
  include/client.h \
  include/content_distribution.h \
  include/decision_policy.h \
  include/error_handling.h \
  include/statistics.h \
  include/ttl_cache.h \
  include/ttl_name_cache.h \
  include/two_ttl_policy.h \
  include/zipf.h \
  include/zipf_sampled.h \
  packets/ccn_data_m.h
$O/src/node/cache/ttl_name_cache.o: src/node/cache/ttl_name_cache.cc \
  include/base_cache.h \
  include/ccnsim.h \
  include/client.h \
  include/error_handling.h \
  include/statistics.h \
  include/ttl_name_cache.h
$O/src/node/cache/two_cache.o: src/node/cache/two_cache.cc \
  include/base_cache.h \
  include/ccnsim.h \
  include/client.h \
  include/two_cache.h
$O/src/node/strategy/MonopathStrategyLayer.o: src/node/strategy/MonopathStrategyLayer.cc \
  include/MonopathStrategyLayer.h \
  include/ccnsim.h \
  include/client.h \
  include/error_handling.h \
  include/strategy_layer.h
$O/src/node/strategy/MultipathStrategyLayer.o: src/node/strategy/MultipathStrategyLayer.cc \
  include/MultipathStrategyLayer.h \
  include/ccnsim.h \
  include/client.h \
  include/error_handling.h \
  include/strategy_layer.h
$O/src/node/strategy/ProbabilisticSplitStrategy.o: src/node/strategy/ProbabilisticSplitStrategy.cc \
  include/MultipathStrategyLayer.h \
  include/ProbabilisticSplitStrategy.h \
  include/base_cache.h \
  include/ccn_interest.h \
  include/ccnsim.h \
  include/client.h \
  include/content_distribution.h \
  include/error_handling.h \
  include/strategy_layer.h \
  include/zipf.h \
  include/zipf_sampled.h \
  packets/ccn_interest_m.h
$O/src/node/strategy/nrr.o: src/node/strategy/nrr.cc \
  include/MonopathStrategyLayer.h \
  include/base_cache.h \
  include/ccn_interest.h \
  include/ccnsim.h \
  include/client.h \
  include/content_distribution.h \
  include/error_handling.h \
  include/nrr.h \
  include/strategy_layer.h \
  include/zipf.h \
  include/zipf_sampled.h \
  packets/ccn_interest_m.h
$O/src/node/strategy/nrr1.o: src/node/strategy/nrr1.cc \
  include/MonopathStrategyLayer.h \
  include/ccn_interest.h \
  include/ccnsim.h \
  include/client.h \
  include/content_distribution.h \
  include/error_handling.h \
  include/nrr1.h \
  include/strategy_layer.h \
  include/zipf.h \
  include/zipf_sampled.h \
  packets/ccn_interest_m.h
$O/src/node/strategy/parallel_repository.o: src/node/strategy/parallel_repository.cc \
  include/MonopathStrategyLayer.h \
  include/ccn_interest.h \
  include/ccnsim.h \
  include/client.h \
  include/content_distribution.h \
  include/error_handling.h \
  include/parallel_repository.h \
  include/strategy_layer.h \
  include/zipf.h \
  include/zipf_sampled.h \
  packets/ccn_interest_m.h
$O/src/node/strategy/random_repository.o: src/node/strategy/random_repository.cc \
  include/MonopathStrategyLayer.h \
  include/ccn_interest.h \
  include/ccnsim.h \
  include/client.h \
  include/content_distribution.h \
  include/error_handling.h \
  include/random_repository.h \
  include/strategy_layer.h \
  include/zipf.h \
  include/zipf_sampled.h \
  packets/ccn_interest_m.h
$O/src/node/strategy/spr.o: src/node/strategy/spr.cc \
  include/MonopathStrategyLayer.h \
  include/ccn_interest.h \
  include/ccnsim.h \
  include/client.h \
  include/content_distribution.h \
  include/error_handling.h \
  include/spr.h \
  include/strategy_layer.h \
  include/zipf.h \
  include/zipf_sampled.h \
  packets/ccn_interest_m.h
$O/src/node/strategy/strategy_layer.o: src/node/strategy/strategy_layer.cc \
  include/ccnsim.h \
  include/client.h \
  include/content_distribution.h \
  include/error_handling.h \
  include/statistics.h \
  include/strategy_layer.h \
  include/zipf.h \
  include/zipf_sampled.h
$O/src/statistics/Tc_Solver.o: src/statistics/Tc_Solver.cc
$O/src/statistics/statistics.o: src/statistics/statistics.cc \
  include/ShotNoiseContentDistribution.h \
  include/always_policy.h \
  include/base_cache.h \
  include/ccn_data.h \
  include/ccnsim.h \
  include/client.h \
  include/client_IRM.h \
  include/content_distribution.h \
  include/core_layer.h \
  include/decision_policy.h \
  include/error_handling.h \
  include/fix_policy.h \
  include/lru_cache.h \
  include/statistics.h \
  include/strategy_layer.h \
  include/ttl_cache.h \
  include/ttl_name_cache.h \
  include/two_lru_policy.h \
  include/two_ttl_policy.h \
  include/zipf.h \
  include/zipf_sampled.h \
  packets/ccn_data_m.h
