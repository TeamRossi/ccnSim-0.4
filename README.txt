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

In this case, the default .ini file ED_TTL-omnetpp.ini (in which each parameter is documented)
will be consider to execute a sample simulation. In particular, a simple  
scenario for the sake of illustration.
Results are produced inside the "results/" folder in the form of .sca files. 


----- EXAMPLE SCRIPT -----
 This version of ccnSim provides, also, an example script:

	runsim_script_ED_TTL.sh {parameters}

used to automatically configure a new .ini file, according to the parameters passed from the command line,
and to collect Key Performance Indicators (KPIs) at the end of the simulations. 
If this script is used, final results are produced under "logs/", "results/", and "infoSim/" directories. 
In particular, a summary of all the collected parameters is produced under "infoSim/ALL_MEASURES_*" file.

!! REMARK: please see the note about the Memory measurement inside the file "runsim_script_ED_TTL.sh".


----- ADDITIONAL SAMPLE SCENARIOS -----
 Furthermore, other sample scenarios, with the correspondent command line parameters, are provided inside:

	run_ED_TTL_scanarios.sh

Note that TTL-based scenarios require files with Tc values to be present inside the folder "Tc_Values";
these files can be produced either by simulating the correspondent scenario with the classic event-driven version
of ccnSim, always using "runsim_script_ED_TTL.sh" (they will be automatically added to the folder),
or by adding them manually. The rationale is that the first line contains the Tc of Node 0, the second line
the Tc of Node 1, and so on.


----- SHOT NOISE MODEL -----
 In order to execute your first simulation with the ShotNoise model, an example file is provided as

	ShotNoiseScenario.txt

An example command that can be used to execute this kind of simulations could be:

	./runsim_script_ED_TTL.sh tree 8 1 spr lce lru 1 1e4 1e4 1e7 1e9 20.0 ShotNoise ShotNoiseContentDistribution 6 0 cold naive 0.75 1	

which executes a simulation with a ShotNoise client and a ShotNoise content distribution, considering LRU caches, 1e7 total contents (i.e., the some of 
all the cardinalities present in the example file), and a Toff = 6.


----- MORE DETAILS -----
 For more details about the output files, please refer to the ccnSim manual.
