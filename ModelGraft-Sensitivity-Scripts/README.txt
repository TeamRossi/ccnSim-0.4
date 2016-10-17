These scripts reproduce all the results presented in "An Hybrid Methodology for the Performance Evaluation of Internet-scale Cache Networks", submitted to Elsevier Computer Networks special issue on "Softwarization and Caching in NGN".

The file "run_ModelGraft_scenarios.sh" contains the complete list of commands associated with the various simulations. 
Each scenario is simulated using the respective "x.sh" file, which, according to input parameters passed through command line, executes the simulation and gathers the main Key Performance Indicators (KPIs).
By default, each scenario is simulated for 10 runs. If you want to simulate less/more runs, please set the correspondent input parameter passed from the command line (their meaning is explained in "run_ModelGraft_scenarios.sh"). 

The run ModelGraft simulations, copy all the scripts in the main folder of ccnSim.    