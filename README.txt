                   ______
		  / _____)_	      
  ____ ____ ____ ( (____ (_)_ __ ____  
 / ___) ___)  _ \ \____ \| | '_ ` _  \ 
( (__( (___| | | |_____) | | | | | | |
 \____)____)_| |_(______/|_|_| |_| |_|


Thank you for joining the ccnSim community!


----- INSTALLATION -----
 To install ccnSim-v4.0, please follow the instructions in INSTALL.txt. 


----- SAMPLE SCENARIO -----
 After having successfully installed ccnSim, you can run your first simulation from the command line 
with the following command:

	./ccnSim -u Cmdenv -f ED_TTL-omnetpp.ini r 0

In this case, the default ED_TTL-omnetpp.ini file will be used to execute a sample simulation. 
Each parameter in ED_TTL-omnetpp.ini is documented with the aim of providing a complete understanding of the features of the simulator and of the relative simple scenario that is simulated. 
At the end of the simulation, log files are written inside the "results/" folder, in the form of .sca files. 


----- EXAMPLE SCRIPT -----
 ccnSim-v4.0 provides, also, an example script which can be used through the following command:

	./runsim_script_ED_TTL.sh {parameters}

This script is meant to automatically configure a new .ini file, according to the parameters passed from the command line, and to collect Key Performance Indicators (KPIs) at the end of the simulations. 
In particular, log files are produced under "logs/", "results/", and "infoSim/" directories. 
A summary file containing all the collected KPIs is produced under "infoSim/ALL_MEASURES_*".


----- ADDITIONAL SAMPLE SCENARIOS -----
 Furthermore, commands to run additional sample scenarios are provided inside:

	run_ED_TTL_scanarios.sh

Note that TTL-based simulations, a.k.a. ModelGraft [1] simulations, require input files containing TTL caches' eviction timers, a.k.a. Tc values, to be present inside the folder "Tc_Values".
These files can be produced either by simulating the correspondent scenario with the classic event-driven (ED) version of ccnSim using always "runsim_script_ED_TTL.sh" (they will be automatically added to the folder), or by creating them manually. 
The rationale is that the first line contains the Tc of Node 0, the second line the Tc of Node 1, and so on. Even if random values are provided, ModelGraft [1] is able to iteratively correct them, thus converging to a consistent state. Despite the Memory usage of ccnSim-v0.4 being extremely optimized, thanks to the use of the Inversion Rejection Sampling technique (see user manual), we suggest performing ED simulations only for small scenarios, i.e., comprising content catalogs with cardinality M < 1e9, owing to CPU and Memory requirements. 


----- SHOT NOISE MODEL -----
 ccnSim-v0.4 comes, also, with an additional model for the generation of content requests with evolving
popularity, know as Shot Noise Model [2].

A reference scenario is already included in order to perform simulations using the ShotNoise model: 

 ./runsim_script_ED_TTL.sh tree 8 1 spr lce lru 1 1e4 1e4 1e7 1e9 20.0 ShotNoise ShotNoiseContentDistribution 6 0 cold naive 0.75 1	

which is based on the features reported inside the "ShotNoiseScenario.txt" file. 
The simulated scenario comprises a ShotNoise client, a ShotNoise content distribution logic, LRU caches with size C = 1e4, a content catalog with M = 1e7 total contents (i.e., the sum of all the cardinalities present in the ShotNoiseScenario.txt file), and an OFF time of Toff = 6.


----- MORE DETAILS -----
 For more details please refer to the ccnSim manual.

----- REFERENCES -----
[1] M. Tortelli, D. Rossi, E. Leonardi, "A Hybrid Methodology for the Performance Evaluation of Internet-scale Cache Networks", Elsevier Computer Networks, Special Issue on Softwarization and Caching in NGN, 2017, DOI: 10.1016/j.comnet.2017.04.006.
[2] S. Traverso et al., Unravelling the Impact of Temporal and Geographical Locality in Content Caching Systems. IEEE Transactions on Multimedia 17(10): 1839-1854 (2015)