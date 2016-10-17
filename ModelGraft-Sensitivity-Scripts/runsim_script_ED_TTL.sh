#!/bin/bash
main=./ccnSim

####### DIRECTORIES ######
resultDir=results
infoDir=infoSim
logDir=logs
tcDir=Tc_Values
##########################

###### READ PARAMETERS from command line ######
parameters=( "$@" )
	
Topology=$(echo ${parameters[0]})			# Topology
NumClients=$(echo ${parameters[1]})			# Number of clients
NumRepos=$(echo ${parameters[2]})			# Number of repos
ForwStr=$(echo ${parameters[3]})			# Forwarding Strategy
MetaCaching=$(echo ${parameters[4]})		# Meta Caching Algorithm
ReplStr=$(echo ${parameters[5]})			# Cache Replacement Strategy, TTL vs ALL (i.e., ED-sim)

if [[ $ReplStr == "ttl" ]]
	then
	SimType="TTL"
else
	SimType="ED"
fi

Alpha=$(echo ${parameters[6]})				# Zipf's Exponent
CacheDim=$(echo ${parameters[7]})			# Cache Dimension (#objects)

if [[ $MetaCaching == "two_lru" ]] || [[ $MetaCaching == "two_ttl" ]]
	then
	NameCacheDim=$(echo ${parameters[8]})		# Name Cache Dimension (2-LRU)
else
	NameCacheDim="0"
fi

TotalCont=$(echo ${parameters[9]})			# Total Number of Contents
TotalReq=$(echo ${parameters[10]})			# Total Number of Requests
Lambda=$(echo ${parameters[11]})			# Request Rate for each client
ClientType=$(echo ${parameters[12]})		# Client Type
ContDistrType=$(echo ${parameters[13]})		# Content Distribution Type
Toff=$(echo ${parameters[14]})				# Toff multiplicative factor (i.e., Ton = k*Toff)

runs=$(echo ${parameters[15]})				# Number of simulated runs

startType=$(echo ${parameters[16]})			# Start Mode ('hot' or 'cold')
fillType=$(echo ${parameters[17]})			# Fill Mode ('model' or 'naive')
checkNodes=$(echo ${parameters[18]})		# Percentage of nodes checked for stabilization (i.e., Yotta <= 1)

down=$(echo ${parameters[19]})				# Downsizing factor.

# If the name of the Tc file is not provided, it will be searched inside the Tc_Values folder, 
# according to the scenario parameters. 
if [[ $SimType == "TTL" ]]
	then
	TcFileTemp=$(echo ${parameters[20]})

	if [ -n "$TcFileTemp" ]					# User provided Tc file
		then
		TcFile=$TcFileTemp
	else
		TcFile=${tcDir}/tc_${Topology}_NumCl_${NumClients}_NumRep_${NumRepos}_FS_${ForwStr}_MC_${MetaCaching}_M_${TotalCont}_R_${TotalReq}_C_${CacheDim}_Lam_${Lambda}.txt
	fi

	if [[ $MetaCaching == "two_ttl" ]]
		then
		TcNameFileTemp=$(echo ${parameters[21]})		
		if [ -n "$TcNameFileTemp" ]						# User provided Tc file for Name Cache
			then
			TcNameFile=$TcNameFileTemp					
		else
			TcNameFile=${tcDir}/tc_${Topology}_NumCl_${NumClients}_NumRep_${NumRepos}_FS_${ForwStr}_MC_${MetaCaching}_M_${TotalCont}_R_${TotalReq}_C_${CacheDim}_Lam_${Lambda}_NameCache.txt
		fi
	else
		TcNameFile="0"
	fi
fi
############################################################


###### COMPUTE StedyTime and other parameters for the TTL-based scenario ######
valueReq=`echo ${TotalReq} | sed -e 's/[eE]+*/\\*10\\^/'`  # Reading scientific notation
steady=$(echo "$valueReq / (${Lambda}*${NumClients})" | bc -l)
steadyINT=${steady/.*}

# TTL-based scenario (i.e., usually downsized). For ED-sim, the downValue should be = 1
downValue=`echo ${down} | sed -e 's/[eE]+*/\\*10\\^/'`  			# Reading scientific notation
TotalContValue=`echo ${TotalCont} | sed -e 's/[eE]+*/\\*10\\^/'`  	# Reading scientific notation
CacheDimValue=`echo ${CacheDim} | sed -e 's/[eE]+*/\\*10\\^/'`  	# Reading scientific notation

CacheDimDown=$(echo "$CacheDimValue / (${downValue})" | bc)
TotalContDown=$(echo "$TotalContValue / (${downValue})" | bc)
TotalReqDown=$(echo "$valueReq / (${downValue})" | bc)

if [[ $SimType == "ED" ]]
	then
	NumCycles=1
fi

echo "*** SIM PARAMETERS ***"
echo "Topology  =  ${Topology}"
echo "NumClients  =  ${NumClients}"
echo "NumRepos  =  ${NumRepos}"
echo "FS  =  ${ForwStr}"
echo "MC  =  ${MetaCaching}"
echo "RS  =  ${ReplStr}"
echo "Sim TYPE  =  ${SimType}"
echo "Alpha  =  ${Alpha}"
echo "Lambda  =  ${Lambda}"
echo "Cache DIM  =  ${CacheDim}"
echo "NameCache DIM  =  ${NameCacheDim}"
echo "Catalog  =  ${TotalCont}"
echo "Requests  =  ${TotalReq}"
echo "Client Type  =  ${ClientType}"
echo "Content Distr Type  =  ${ContDistrType}"
echo "Toff  =  ${Toff}"
echo "Start Type  =  ${startType}" 
echo "Fill Type  =  ${fillType}"
echo "Yotta  =  ${checkNodes}"
echo "#Runs  =  ${runs}"
echo "Steady Time  =  ${steadyINT}"

echo "Downsizing factor  =  ${down}"


if [[ SimType == "TTL" ]]
	then
	echo "Tc file  =  ${TcFile}"
	echo "Tc Name file  =  ${TcNameFile}"
fi
##################################################################


###### START of each LINE that needs to be replaced inside the .ini file  ######
numClientsLS="**.num_clients"
numReposLS="**.num_repos"

netLS="network"
fwLS="**.FS"
mcLS="**.DS"
rsLS="**.RS"
alphaLS="**.alpha"
cacheDimLS="**.C"
nameCacheDimLS="**.NC"
totContLS="**.objects"
lambdaLS="**.lambda"
clientTypeLS="**.client_type"
contentDistrTypeLS="**.content_distribution_type"
catAggrLS="**.perc_aggr"
steadyLS="**.steady"
toffLS="**.toff_mult_factor"

startTypeLS="**.start_mode"
fillModeLS="**.fill_mode"
checkedNodesLS="**.partial_n"

downLS="**.downsize"

tcLS="**.tc_file"
tcNameLS="**.tc_name_file"

outputVectorIniLS="output-vector-file"
outputScalarIniLS="output-scalar-file"
##########################################################################################


####### MODIFY the .INI file ###########
now=$(date +"%H_%M_%S")		# Current Time
	
iniFileMaster=ED_TTL-omnetpp.ini
iniFileFinal=${Topology}_${ContDistrType}_${now}.ini
	
`cp ${iniFileMaster} ${iniFileFinal}`          # 'iniFileFinal' is the .ini file this script will be working on.

#Replacing Parameters inside the 'iniFileFinal'
# Topology
`awk -v v1="${netLS}" -v v2="${Topology}" '$1==v1{$5='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

# Num Clients
`awk -v v1="${numClientsLS}" -v v2="${NumClients}" '$1==v1{$5='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

# Num Repos
`awk -v v1="${numReposLS}" -v v2="${NumRepos}" '$1==v1{$5='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

# Forwarding Strategy
`awk -v v1="${fwLS}" -v v2="${ForwStr}" '$1==v1{$6='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

# Meta Caching
`awk -v v1="${mcLS}" -v v2="${MetaCaching}" '$1==v1{$6='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

# Replacement Strategy
`awk -v v1="${rsLS}" -v v2="${ReplStr}" '$1==v1{$6='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

# Alpha
`awk -v v1="${alphaLS}" -v v2="${Alpha}" '$1==v1{$5='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`  

# Cache Size
`awk -v v1="${cacheDimLS}" -v v2="${CacheDim}" '$1==v1{$5='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}` 

# Name Cache Size (2-LRU)
`awk -v v1="${nameCacheDimLS}" -v v2="${NameCacheDim}" '$1==v1{$5='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

# Total Contents
`awk -v v1="${totContLS}" -v v2="${TotalCont}" '$1==v1{$5='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

# Lambda
`awk -v v1="${lambdaLS}" -v v2="${Lambda}" '$1==v1{$5='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

# Client Type
`awk -v v1="${clientTypeLS}" -v v2="${ClientType}" '$1==v1{$5='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

# Content Distribution Type
# If the client type is 'IRM', comment the content distribution string in order to simulate the default one
if [[ $ClientType == "IRM"  ||  $ClientType == "Window" ]]
	then
	`awk -v v1="${contentDistrTypeLS}" -v v2="#${contentDistrTypeLS}" '$1==v1{$1='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
   	`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`
else
	`awk -v v1="${contentDistrTypeLS}" -v v2="${ContDistrType}" '$1==v1{$5='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
   	`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`
fi

# Toff multiplicative factor
`awk -v v1="${toffLS}" -v v2="${Toff}" '$1==v1{$6='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

# Steady Time
`awk -v v1="${steadyLS}" -v v2="${steadyINT}" '$1==v1{$5='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

# Start Mode
`awk -v v1="${startTypeLS}" -v v2="${startType}" '$1==v1{$5='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

# Fill Mode
`awk -v v1="${fillModeLS}" -v v2="${fillType}" '$1==v1{$5='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

# Checked Nodes
`awk -v v1="${checkedNodesLS}" -v v2="${checkNodes}" '$1==v1{$5='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

# Downsizing Factor
`awk -v v1="${downLS}" -v v2="${down}" '$1==v1{$5='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

if [[ $SimType == "TTL" ]]
	then
	# Tc File Name
	`awk -v v1="${tcLS}" -v v2="${TcFile}" '$1==v1{$6='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
	`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

	# Tc Name Cache File Name
	`awk -v v1="${tcNameLS}" -v v2="${TcNameFile}" '$1==v1{$6='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
	`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`
fi
	
# Output Strings
outString=${SimType}_T_${Topology}_NumCl_${NumClients}_NumRep_${NumRepos}_FS_${ForwStr}_MC_${MetaCaching}_RS_${ReplStr}_C_${CacheDim}_NC_${NameCacheDim}_M_${TotalCont}_Req_${TotalReq}_Lam_${Lambda}_A_${Alpha}_CT_${ClientType}_ToffMult_${Toff}_Start_${startType}_Fill_${fillType}_ChNodes_${checkNodes}_Down_${down}

tcOutString=tc_${Topology}_NumCl_${NumClients}_NumRep_${NumRepos}_FS_${ForwStr}_MC_${MetaCaching}_M_${TotalCont}_R_${TotalReq}_C_${CacheDim}_Lam_${Lambda}.txt
tcOutStringNameCache=tc_${Topology}_NumCl_${NumClients}_NumRep_${NumRepos}_FS_${ForwStr}_MC_${MetaCaching}_M_${TotalCont}_R_${TotalReq}_C_${CacheDim}_Lam_${Lambda}_NameCache.txt

`awk -v v1="${outputVectorIniLS}" -v v2="\$"{resultdir}"/${outString}_run=\$"{repetition}".vec" '$1==v1{$3='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`

`awk -v v1="${outputScalarIniLS}" -v v2="\$"{resultdir}"/${outString}_run=\$"{repetition}".sca" '$1==v1{$3='v2'}{print $0}' ${iniFileFinal} > ${iniFileFinal}_temp.ini`
`mv ${iniFileFinal}_temp.ini ${iniFileFinal}`
##########################################################################################

############# EXECUTE SIMULATIONS ##################
for i in `seq 0 $runs`
do
   	/usr/bin/time -f "\n%E \t elapsed real time \n%U \t total CPU seconds used (user mode) \n%S \t total CPU seconds used by the system  on behalf of the process \n%M \t memory (max resident set size) [kBytes] \n%x \t exit status" -o ${infoDir}/Info_${outString}_run\=${i}.txt $main -u Cmdenv -f $iniFileFinal -r $i > $logDir/${outString}_run\=${i}.out 2>&1
done
##########################################################################################


###### COMPUTE METRICS ######
NumNodes=$(grep "Num Total Nodes" ${logDir}/${outString}_run\=0.out | awk '{print $4}')
cat ${resultDir}/${outString}* | grep "p_hit " | awk '{print $4}' > ${resultDir}/HIT_${outString}_RUNS
if [[ $SimType == "TTL" ]]
	then
	# Print the p_hit and the CPU time at the last stabilization
	for i in `seq 0 $runs`
	do
		grep "MEAN HIT PROB AFTER STABILIZATION" ${logDir}/${outString}_run\=${i}.out | awk '{print $9}' | tail -n 1 >> ${resultDir}/HIT_STABLE_${outString}_RUNS
		grep "Execution time of the STABILIZATION" ${logDir}/${outString}_run\=${i}.out | awk '{print $7}' | tail -n 1 >> ${resultDir}/CPU_STABLE_${outString}_RUNS
	done
	grep -h "SIMULATION ENDED AT CYCLE" ${logDir}/${outString}* | awk '{print $6}' > ${resultDir}/SIM_CYCLES_${outString}_RUNS
else
	grep -h "MEAN HIT PROB AFTER STABILIZATION" ${logDir}/${outString}* | awk '{print $9}' > ${resultDir}/HIT_STABLE_${outString}_RUNS
	grep -h "Execution time of the STABILIZATION" ${logDir}/${outString}* | awk '{print $7}' > ${resultDir}/CPU_STABLE_${outString}_RUNS
fi

cat ${resultDir}/${outString}* | grep "hdistance " | awk '{print $4}' > ${resultDir}/DISTANCE_${outString}_RUNS
cat ${resultDir}/${outString}* | grep "interests " | awk '{print $4}' > ${resultDir}/Interests_${outString}_RUNS
cat ${resultDir}/${outString}* | grep "data " | awk '{print $4}' > ${resultDir}/Data_${outString}_RUNS
paste -d\; ${resultDir}/Interests_${outString}_RUNS ${resultDir}/Data_${outString}_RUNS | awk -F\; '{print $1+$2}' OFS=\; > ${resultDir}/LOAD_${outString}_RUNS

if [[ $SimType == "ED" ]]
	then
	grep "Evaluated Tc:" ${logDir}/${outString}_run\=0.out | awk '{print $6}' > ${tcDir}/${tcOutString}
	if [[ $MetaCaching == "two_lru" ]]
		then
		grep "NAME CACHE LRU Tc:" ${logDir}/${outString}_run\=0.out | awk '{print $8}' > ${tcDir}/${tcOutStringNameCache}
	fi
fi

awk '{sum+=$1} END { print sum/NR}' ${resultDir}/HIT_${outString}_RUNS >> ${resultDir}/HIT_${outString}
awk '{sum+=$1} END { print sum/NR}' ${resultDir}/HIT_STABLE_${outString}_RUNS >> ${resultDir}/HIT_STABLE_${outString}
awk '{sum+=$1} END { print sum/NR}' ${resultDir}/CPU_STABLE_${outString}_RUNS >> ${resultDir}/CPU_STABLE_${outString}
awk '{sum+=$1} END { print sum/NR}' ${resultDir}/DISTANCE_${outString}_RUNS >> ${resultDir}/DISTANCE_${outString}
awk '{sum+=$1} END { print sum/NR}' ${resultDir}/LOAD_${outString}_RUNS >> ${resultDir}/LOAD_${outString}
if [[ $SimType == "TTL" ]]
	then
	awk '{sum+=$1} END { print sum/NR}' ${resultDir}/SIM_CYCLES_${outString}_RUNS >> ${resultDir}/SIM_CYCLES_${outString}
fi

awk '{x[NR]=$1; s+=$1} END{a=s/NR; for (i in x){ss += (x[i]-a)^2} sd = sqrt(ss/NR) ;conf=1.860*(sd/sqrt(NR)); print sd; print conf}' ${resultDir}/HIT_${outString}_RUNS >> ${resultDir}/HIT_${outString}
awk '{x[NR]=$1; s+=$1} END{a=s/NR; for (i in x){ss += (x[i]-a)^2} sd = sqrt(ss/NR) ;conf=1.860*(sd/sqrt(NR)); print sd; print conf}' ${resultDir}/HIT_STABLE_${outString}_RUNS >> ${resultDir}/HIT_STABLE_${outString}
awk '{x[NR]=$1; s+=$1} END{a=s/NR; for (i in x){ss += (x[i]-a)^2} sd = sqrt(ss/NR) ;conf=1.860*(sd/sqrt(NR)); print sd; print conf}' ${resultDir}/CPU_STABLE_${outString}_RUNS >> ${resultDir}/CPU_STABLE_${outString}
awk '{x[NR]=$1; s+=$1} END{a=s/NR; for (i in x){ss += (x[i]-a)^2} sd = sqrt(ss/NR) ;conf=1.860*(sd/sqrt(NR)); print sd; print conf}' ${resultDir}/DISTANCE_${outString}_RUNS >> ${resultDir}/DISTANCE_${outString}
awk '{x[NR]=$1; s+=$1} END{a=s/NR; for (i in x){ss += (x[i]-a)^2} sd = sqrt(ss/NR) ;conf=1.860*(sd/sqrt(NR)); print sd; print conf}' ${resultDir}/LOAD_${outString}_RUNS >> ${resultDir}/LOAD_${outString}
if [[ $SimType == "TTL" ]]
	then
	awk '{x[NR]=$1; s+=$1} END{a=s/NR; for (i in x){ss += (x[i]-a)^2} sd = sqrt(ss/NR) ;conf=1.860*(sd/sqrt(NR)); print sd; print conf}' ${resultDir}/SIM_CYCLES_${outString}_RUNS >> ${resultDir}/SIM_CYCLES_${outString}
fi
##############################################

###### COMPUTE PERFORMANCE ######
cat $infoDir/Info_${outString}* | grep "elapsed" | awk '{print $1}' > $infoDir/Elapsed_${outString}_RUNS
cat $infoDir/Info_${outString}* | grep "user" | awk '{print $1}' > $infoDir/CPU_${outString}_RUNS
cat $infoDir/Info_${outString}* | grep "memory" | awk '{print $1}' > $infoDir/Memory_${outString}_RUNS

awk '{sum+=$1} END { print sum/NR}' ${infoDir}/CPU_${outString}_RUNS >> ${infoDir}/CPU_${outString}
awk '{sum+=$1} END { print sum/NR}' ${infoDir}/Memory_${outString}_RUNS >> ${infoDir}/Memory_${outString}

awk '{x[NR]=$1; s+=$1} END{a=s/NR; for (i in x){ss += (x[i]-a)^2} sd = sqrt(ss/NR) ;conf=1.860*(sd/sqrt(NR)); print sd; print conf}' ${infoDir}/CPU_${outString}_RUNS >> ${infoDir}/CPU_${outString}
awk '{x[NR]=$1; s+=$1} END{a=s/NR; for (i in x){ss += (x[i]-a)^2} sd = sqrt(ss/NR) ;conf=1.860*(sd/sqrt(NR)); print sd; print conf}' ${infoDir}/Memory_${outString}_RUNS >> ${infoDir}/Memory_${outString}

# Gather all the metrics (mean values) in one file
hitStabStr=$(head -n 1 ${resultDir}/HIT_STABLE_${outString})
hitEndStr=$(head -n 1 ${resultDir}/HIT_${outString})
distanceStr=$(head -n 1 ${resultDir}/DISTANCE_${outString})
load_str=$(head -n 1 ${resultDir}/LOAD_${outString})
cpuStabStr=$(head -n 1 ${resultDir}/CPU_STABLE_${outString})
cpuEndStr=$(head -n 1 ${infoDir}/CPU_${outString})
memoryStr=$(head -n 1 ${infoDir}/Memory_${outString})
if [[ $SimType == "TTL" ]]
	then
	NumCycles=$(head -n 1 ${resultDir}/SIM_CYCLES_${outString})
fi
#CPU stab in seconds
cpuStab=`echo "scale=2; $cpuStabStr / 1000" | bc`
#Memory in MB
## IMPORTANT : in Linux distributions previous to the 14.04, the Memory gathered with "time" does not correspond to the actual memory
##                         used by the program; there is, indeed, a multiplicative factor of x4. In that case, the value $memoryStr 
##                         should be divided by 4096 in order to express the exact memory in MBytes. Please check your distribution.
#memory=`echo "scale=2; $memoryStr / 1024" | bc`


header_str="Topo\tNodes\tClients\tRepos\tAlpha\tLambda\tMC\tRP\tFS\tM\tC\tR\tDelta\tM'\tC'\tR'\tYotta\tp_hit_stab\tp_hit_end\thit_dist\tMem[MB]\tCPU_stab[s]\tCPU_end[s]\tCycles\tSimType\tStartType\tFillType\n"
final_str="${Topology}\t${NumNodes}\t${NumClients}\t${NumRepos}\t${Alpha}\t${Lambda}\t${MetaCaching}\t${ReplStr}\t${ForwStr}\t${TotalCont}\t${CacheDim}\t${TotalReq}\t${down}\t${TotalContDown}\t${CacheDimDown}\t${TotalReqDown}\t${checkNodes}\t${hitStabStr}\t${hitEndStr}\t${distanceStr}\t${memory}\t${cpuStab}\t${cpuEndStr}\t${NumCycles}\t${SimType}\t${startType}\t${fillType}\n"

echo -e $header_str > ${infoDir}/ALL_MEASURES_${outString}
echo -e $final_str >> ${infoDir}/ALL_MEASURES_${outString}
###################################################################
