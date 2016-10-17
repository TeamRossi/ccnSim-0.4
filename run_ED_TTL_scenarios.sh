#!/bin/bash

#### INFO #####

#-Parameters:
#Topo #Clients #Repos #FS #MC #RS #Alpha #CacheDim #NameCacheDim #Catalog #Req #Lambda #Client #ContDistr #Toff #Runs #Start #Fill #Yotta #Down (#TcFile #TcNameFile)


# Num of simulated runs n = #Runs + 1.


######## TOY CASES ########
## ED-SIM ##
#./runsim_script_ED_TTL.sh single_cache 1 1 spr lce lru 1 1e4 1e4 1e6 1e6 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh single_cache 1 1 spr lce lru 1 1e4 1e4 1e7 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh single_cache 1 1 spr lce lru 1 1e5 1e5 1e7 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh single_cache 1 1 spr lce lru 1 1e6 1e6 1e7 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh single_cache 1 1 spr lce lru 1 1e4 1e4 1e8 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh single_cache 1 1 spr lce lru 1 1e3 1e3 1e6 1e4 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh single_cache 1 1 spr lce lru 1 1e5 1e5 1e8 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh tandem_net 1 1 spr lce lru 1 1e4 1e4 1e6 1e6 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh tandem_net 1 1 spr lce lru 1 1e4 1e4 1e7 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh tandem_net 1 1 spr lce lru 1 1e5 1e5 1e7 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh tandem_net 1 1 spr lce lru 1 1e6 1e6 1e7 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh tandem_net 1 1 spr lce lru 1 1e4 1e4 1e8 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh tandem_net 1 1 spr lce lru 1 1e3 1e3 1e6 1e4 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh tandem_net 1 1 spr lce lru 1 1e5 1e5 1e8 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1

## TTL ##
#./runsim_script_ED_TTL.sh single_cache 1 1 spr lce ttl 1 1e4 1e4 1e6 1e6 20.0 IRM IRM 0 0 cold naive 0.75 1e3
#./runsim_script_ED_TTL.sh single_cache 1 1 spr lce ttl 1 1e4 1e4 1e7 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1e3
#./runsim_script_ED_TTL.sh single_cache 1 1 spr lce ttl 1 1e5 1e5 1e7 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1e3
#./runsim_script_ED_TTL.sh single_cache 1 1 spr lce ttl 1 1e6 1e6 1e7 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1e3
#./runsim_script_ED_TTL.sh single_cache 1 1 spr lce ttl 1 1e4 1e4 1e8 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1e3
#./runsim_script_ED_TTL.sh single_cache 1 1 spr lce ttl 1 1e3 1e3 1e6 1e4 20.0 IRM IRM 0 0 cold naive 0.75 1e2
#./runsim_script_ED_TTL.sh single_cache 1 1 spr lce ttl 1 1e5 1e5 1e8 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1e4
#./runsim_script_ED_TTL.sh tandem_net 1 1 spr lce ttl 1 1e4 1e4 1e6 1e6 20.0 IRM IRM 0 0 cold naive 0.75 1e3
#./runsim_script_ED_TTL.sh tandem_net 1 1 spr lce ttl 1 1e4 1e4 1e7 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1e3
#./runsim_script_ED_TTL.sh tandem_net 1 1 spr lce ttl 1 1e5 1e5 1e7 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1e3
#./runsim_script_ED_TTL.sh tandem_net 1 1 spr lce ttl 1 1e6 1e6 1e7 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1e3
#./runsim_script_ED_TTL.sh tandem_net 1 1 spr lce ttl 1 1e4 1e4 1e8 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1e3
#./runsim_script_ED_TTL.sh tandem_net 1 1 spr lce ttl 1 1e3 1e3 1e6 1e4 20.0 IRM IRM 0 0 cold naive 0.75 1e2
#./runsim_script_ED_TTL.sh tandem_net 1 1 spr lce ttl 1 1e5 1e5 1e8 1e7 20.0 IRM IRM 0 0 cold naive 0.75 1e4


######## LARGE SCENARIOS ########
## ED-SIM ##
#./runsim_script_ED_TTL.sh tree 8 1 spr lcd lru 1 1e5 1e5 1e8 1e8 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh tree 8 1 spr fix0.1 lru 1 1e5 1e5 1e8 1e8 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh ndntestbed_26 8 1 spr two_lru_lru lru 1 1e5 1e5 1e8 1e8 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh ndntestbed_26_rand 15 1 spr two_lru lru 1 1e5 1e5 1e8 1e8 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh ndntestbed_26_rand 15 3 spr two_lru lru 1 1e5 1e5 1e8 1e8 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh tree 8 1 spr lce lru 1 1e5 1e5 1e8 1e8 20.0 IRM IRM 0 0 cold naive 0.75 1
#./runsim_script_ED_TTL.sh tree 8 1 spr lce lru 1 100000 100000 1e8 1e8 20.0 IRM IRM 0 0 hot model 0 1

## TTL ##
./runsim_script_ED_TTL.sh tree 8 1 spr lce ttl 1 1e5 1e5 1e8 1e8 20.0 IRM IRM 0 0 cold naive 0.75 1e4
#./runsim_script_ED_TTL.sh tree 8 1 spr lcd ttl 1 1e5 1e5 1e8 1e8 20.0 IRM IRM 0 0 cold naive 0.75 1e4
#./runsim_script_ED_TTL.sh tree 8 1 spr fix0.1 ttl 1 1e5 1e5 1e8 1e8 20.0 IRM IRM 0 0 cold naive 0.75 1e4
#./runsim_script_ED_TTL.sh ndntestbed_26 8 1 spr two_lru ttl 1 1e5 1e5 1e8 1e8 20.0 IRM IRM 0 0 cold naive 0.75 1e4
#./runsim_script_ED_TTL.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 0 cold naive 0.75 1e5
#./runsim_script_ED_TTL.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 0 cold naive 0.75 1e5
#./runsim_script_ED_TTL.sh tree 8 1 spr lcd ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 0 cold naive 0.75 1e5
#./runsim_script_ED_TTL.sh tree 8 1 spr two_ttl ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 0 cold naive 0.75 1e5
#./runsim_script_ED_TTL.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e10 1e10 20.0 IRM IRM 0 0 cold naive 0.75 1e5
#./runsim_script_ED_TTL.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e11 1e11 20.0 IRM IRM 0 0 cold naive 0.75 1e5
