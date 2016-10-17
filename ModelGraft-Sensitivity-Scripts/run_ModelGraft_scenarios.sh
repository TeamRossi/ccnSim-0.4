#!/bin/bash

#-Parameters:
#Topo #Clients #Repos #FS #MC #RS #Alpha #CacheDim #NameCacheDim #Catalog #Req #Lambda #Client #ContDistr #Toff #Runs #Start #Fill #Yotta #Down (#CV #Cons) (#TcFile #TcNameFile)


### ModelGraft Validation ###
# ED-SIM #
./runsim_script_ED_TTL.sh tree 8 1 spr lce lru 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1
./runsim_script_ED_TTL.sh tree 8 1 spr fix0.1 lru 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1
./runsim_script_ED_TTL.sh tree 8 1 spr two_lru lru 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1
# TTL #
./runsim_script_ED_TTL.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5
./runsim_script_ED_TTL.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5
./runsim_script_ED_TTL.sh tree 8 1 spr two_ttl ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5


### Internet-scale ###
# TTL #
./runsim_script_ED_TTL.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e10 1e10 20.0 IRM IRM 0 9 cold naive 0.75 1e5
./runsim_script_ED_TTL.sh cdn_67 32 5 spr lce ttl 1 1e7 1e7 1e11 1e11 20.0 IRM IRM 0 9 cold naive 0.75 1e6


### Tc Sensitivity ###
# LCE #
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCE/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_lce_M_1e9_R_1e9_C_1e6_Lam_20.0 true																															
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCE/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_lce_M_1e9_R_1e9_C_1e6_Lam_20.0-rand100 rand100
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCE/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_lce_M_1e9_R_1e9_C_1e6_Lam_20.0-rand10 rand10	
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCE/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_lce_M_1e9_R_1e9_C_1e6_Lam_20.0-rand5 rand5
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCE/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_lce_M_1e9_R_1e9_C_1e6_Lam_20.0-rand2 rand2
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCE/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_lce_M_1e9_R_1e9_C_1e6_Lam_20.0-rand100div rand100div
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCE/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_lce_M_1e9_R_1e9_C_1e6_Lam_20.0-rand10div rand10div
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCE/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_lce_M_1e9_R_1e9_C_1e6_Lam_20.0-rand5div rand5div
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCE/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_lce_M_1e9_R_1e9_C_1e6_Lam_20.0-rand2div rand2div
# LCP(0.1) #
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCP/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_fix0.1_M_1e9_R_1e9_C_1e6_Lam_20.0 true																															
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCP/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_fix0.1_M_1e9_R_1e9_C_1e6_Lam_20.0-rand100 rand100
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCP/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_fix0.1_M_1e9_R_1e9_C_1e6_Lam_20.0-rand10 rand10	
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCP/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_fix0.1_M_1e9_R_1e9_C_1e6_Lam_20.0-rand5 rand5
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCP/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_fix0.1_M_1e9_R_1e9_C_1e6_Lam_20.0-rand2 rand2
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCP/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_fix0.1_M_1e9_R_1e9_C_1e6_Lam_20.0-rand100div rand100div
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCP/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_fix0.1_M_1e9_R_1e9_C_1e6_Lam_20.0-rand10div rand10div
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCP/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_fix0.1_M_1e9_R_1e9_C_1e6_Lam_20.0-rand5div rand5div
./runsim_script_ED_TTL_Tc_Sensitivity.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 Tc_Values/Tc_Sensitivity/LCP/tc_tree_NumCl_8_NumRep_1_FS_spr_MC_fix0.1_M_1e9_R_1e9_C_1e6_Lam_20.0-rand2div rand2div



### Delta Sensitivity ###
# LCP(0.1) #
./runsim_script_ED_TTL.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e2
./runsim_script_ED_TTL.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e3
./runsim_script_ED_TTL.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e4
./runsim_script_ED_TTL.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e6


### Y Sensitivity ###
# LCE #
./runsim_script_ED_TTL.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 1 1e5
./runsim_script_ED_TTL.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.95 1e5
./runsim_script_ED_TTL.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.9 1e5
./runsim_script_ED_TTL.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.5 1e5


### Alpha Sensitivity ###
# ED-SIM #
./runsim_script_ED_TTL_alpha_diff.sh tree 8 1 spr lce lru 0.8 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1
./runsim_script_ED_TTL_alpha_diff.sh tree 8 1 spr lce lru 1.2 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1
# TTL #
./runsim_script_ED_TTL_alpha_diff.sh tree 8 1 spr ttl lru 0.8 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5
./runsim_script_ED_TTL_alpha_diff.sh tree 8 1 spr ttl lru 1.2 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5

### CV and Cons Sensitivity ###
## Sensitivity on CV Threshold ##
# ED - LCE #
./runsim_script_ED_TTL_CV_and_Cons_Sensitivity.sh tree 8 1 spr lce lru 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1 0.005 0.1
./runsim_script_ED_TTL_CV_and_Cons_Sensitivity.sh tree 8 1 spr lce lru 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1 0.01 0.1
./runsim_script_ED_TTL_CV_and_Cons_Sensitivity.sh tree 8 1 spr lce lru 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1 0.05 0.1
./runsim_script_ED_TTL_CV_and_Cons_Sensitivity.sh tree 8 1 spr lce lru 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1 0.1 0.1
# ED - LCP(0.1) #
./runsim_script_ED_TTL_CV_and_Cons_Sensitivity.sh tree 8 1 spr fix0.1 lru 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1 0.005 0.1
./runsim_script_ED_TTL_CV_and_Cons_Sensitivity.sh tree 8 1 spr fix0.1 lru 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1 0.01 0.1
./runsim_script_ED_TTL_CV_and_Cons_Sensitivity.sh tree 8 1 spr fix0.1 lru 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1 0.05 0.1
./runsim_script_ED_TTL_CV_and_Cons_Sensitivity.sh tree 8 1 spr fix0.1 lru 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1 0.1 0.1

# TTL - LCE #
./runsim_script_ED_TTL_CV_and_Cons_Sensitivity.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 0.005 0.1
./runsim_script_ED_TTL_CV_and_Cons_Sensitivity.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 0.01 0.1
./runsim_script_ED_TTL_CV_and_Cons_Sensitivity.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 0.05 0.1
./runsim_script_ED_TTL_CV_and_Cons_Sensitivity.sh tree 8 1 spr lce ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 0.1 0.1
# TTL - LCP(0.1) #
./runsim_script_ED_TTL_CV_and_Cons_Sensitivity.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 0.005 0.1
./runsim_script_ED_TTL_CV_and_Cons_Sensitivity.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 0.01 0.1
./runsim_script_ED_TTL_CV_and_Cons_Sensitivity.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 0.05 0.1
./runsim_script_ED_TTL_CV_and_Cons_Sensitivity.sh tree 8 1 spr fix0.1 ttl 1 1e6 1e6 1e9 1e9 20.0 IRM IRM 0 9 cold naive 0.75 1e5 0.1 0.1