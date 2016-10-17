#ifdef __cplusplus
extern "C" {
#endif
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>



// Supported cache replacement strategies: LRU, qLRU, FIFO, RANDOM. (Check for FIFO and RANDOM)
typedef enum {LRU,qLRU} policy;   
policy cache_pol;

int STAGES;                 // Levels of the tree topology.
int CATALOGUE;              // Catalog Cardinality + 1.
int TREE_DEG;               // Degree of the tree topology.
int MAXITER;                // Maximum number of iteration to calculate the characteristic time (Tc).
int DETAILEDPRINT;          // Print the Hit probability per each content.
double Q;                   // Caching probability of the 'qLRU' meta-caching algorithm.


/*
*	Definition of used structures.
*
*		- lambda: matrix[CATALOGUE][STAGES] indicating the request arrival rate per each content at each stage (i.e., level of the tree).
*		- pIn: matrix[CATALOGUE][STAGES] indicating the Pin probability per each content at each stage.
*		- Tc: vector[STAGES] indicating the "characteristic times" of the caches at each stage.
*		- cSize: vector[STAGES] indicating the size of the caches at each stage. 
*		- pHit: vector[STAGES] indicating the average hit probability at each stage.
*		- sumzipf: vector[STAGES] containing the normalization constant (i.e., sum(i=1:M)(1/i^alpha)) of the Zipf distribution at each stage.
*/
double** lambda; 
double** pIn;

double* Tc;
double* cSize;
double* pHit;
double* sumzipf;


// Calculate the normalization constant of the Zipf distribution according to the specified stage. 
double initiZipf(double alpha, long CATALOG, int stage)
{
	double sum;
 	long k;
 	sum=0.0;
 	if(stage==0)
 	{
 		for (k=1; k<=CATALOG; k++)
	    	sum += pow((double )k, -alpha);
  }
  else
  {
    	for (k=1; k<=CATALOG; k++)
	    	sum += lambda[k][stage];
    }
	return sum;       
}    

/* Calculate the Zipf popularity for the specified content.
*
*	Parameters:
*		- k: content ID;
*		- alpha: Zipf's exponent.
*/
double zipf_popularity(long k, double alpha)
{
  return  pow((double )k,-alpha)/sumzipf[0]; 
}

/* Compute the request arrival rate according to the specified content and stage.
*
*	Parameters:
*		- Lambda: aggregate exogenous request rate;
*		- k: content ID;
*		- stage: level of the tree;
*		- alpha: Zipf's exponent.
*/
double compute_lambda(double Lambda, long k, int stage, double alpha) 
{
	double res;
    if(stage==0)  
    	res = Lambda*zipf_popularity(k,alpha);
    else  
    	res = ((double)TREE_DEG)*lambda[k][stage-1]*(1.0- pIn[k][stage-1]);  // Miss streams at lower nodes in the tree.
	return res;
}

/* Compute the Pin probability according to the specified content and stage.
*
*	Parameters:
*		- k: content ID;
*		- stage: level of the tree;
*/
double  compute_pin(long k, long stage) 
{
	double a = 1.0 - exp(-lambda[k][stage]*Tc[stage]);
  	if(cache_pol==LRU)  
  		return  a;
	//  if((cache_pol==FIFO)||((cache_pol==RANDOM)))   return Lambda*zipf_popularity(k,alpha)*Tc/(1.0+ Lambda*zipf_popularity(k,alpha)*Tc);
  	else if(cache_pol==qLRU)
  	{
  		double q = Q;
  		return  a*q/(1.0-a*(1-q));
  	}
  	else
  	  	printf("Caching policy NOT implemented!\n");
  	exit(0);    
}



/* Compute the size of the cache as the sum of the content hit probability.
*
*	Parameters:
*		- stage: level of the tree;
*		- CATALOG: cardinality of the catalog.
*/
double  compute_cache_dim(int stage, long CATALOG)
{
	long  k;
    double cacheSize;
    cacheSize = 0.0;
    for (k=1; k<=CATALOG; k++)
    {
    	pIn[k][stage] = compute_pin(k, stage); 
	  	//printf( "k=%ld, s=%d, lambda=%le,  pIn=%le\n", k,stage, lambda[k][stage], pIn[k][stage]);
	  	cacheSize += pIn[k][stage];
    }
	return cacheSize;
}

/* Compute the pHit probability of a content at a particular stage.
*
*	Parameters:
*		- k: content ID.
*		- stage: level of the tree;
*/
double  compute_phit_cont(long k, int stage) 
{
	double  a,b;
  	double  q = Q;
  	double TC2,TC1;
  	if(cache_pol == LRU)
  	{
  		b = 0.0;
     	TC2 = Tc[stage];
     	if(stage > 0)
     	{
     		TC1 = Tc[stage-1];
         	b = lambda[k][stage]*TC2;
     	}
     	else 
     		TC1 = 0.0;
     	if(TC2 > TC1)
     		a = lambda[k][stage]*(TC2-TC1);
     	else
     		a = 0.0;
      if (stage > 0)  
      {
      	if (TREE_DEG == 1) 
      		return 1.0 - exp (-a);
      	else 
      		return 1.0 - exp (-b);		// Only the TC2 is taken into account for TREE_DEG > 1.
      }
      else
      	return 1.0 - exp (-a);
    }
	//if((cache_pol==FIFO)||((cache_pol==RANDOM))) {
	//   if(TC2>TC1)  return  lambda*(TC2-TC1)/(1.0 + lambda*(TC2-TC1)); else return 0.0;}
    else if(cache_pol == qLRU)
    {
    	TC2 = Tc[stage];
      if(stage>0)
       	TC1 = Tc[stage-1];
      else
       	TC1 = 0.0;
      if(TC2 > TC1)
      {
       	if(TREE_DEG == 1) 
			    a = (1.0-exp(-lambda[k][stage]*(TC2-TC1)))*exp(-lambda[k][stage]*(TC1))+ 
             	(1.0/TREE_DEG*(1.0-q)+(TREE_DEG-1.0)/TREE_DEG)*(1.0-exp(-lambda[k][stage]*TC2));
		    else 
		     a = 1.0-exp(-lambda[k][stage]*TC2);
		  }
		  else
    	{
    		if(TREE_DEG == 1)
    			a = (1.0-q)*(1.0-exp(-lambda[k][stage]*TC2));
    		else a = 1.0-exp(-lambda[k][stage]*TC2);
    	}
    	b = 1.0-exp(-lambda[k][stage]*TC2);
    	if(stage > 0)
    		return a*q/(1.0- a*(1.0-q));
    	else
    		return b*q/(1.0-b*(1.0-q));
    }
    else
    	printf("Caching policy not implemented!\n");
    exit(0);    
}


/* Compute the average Hit probability of the cache at the specified stage (i.e., sum(p_k * p_hit(k)).
*
*	Parameters:
*		- k: content ID.
*		- stage: level of the tree;
*/
double compute_avg_phit(long CATALOG, int stage, double alpha)
{
	long k;
    double phit = 0.0;
    double phitc; 
     
    for (k=1; k<=CATALOG; k++)
    {
    	phitc = compute_phit_cont(k, stage);
       	if(stage > 0) 
       		phit += lambda[k][stage]*phitc/sumzipf[stage]; 
        else 
        	phit += zipf_popularity(k,alpha)*compute_phit_cont(k, stage);
      	if(DETAILEDPRINT) 
      		printf(" stage=%d phit(%ld)=%le\n",stage, k, phitc);
    }
    return phit;
}

double compute_Tc_single_Approx(double cSizeTarg, double alphaVal, long catCard, float** reqRates, int colIndex, const char* dp, double q)
{
  double Tc, Tc1, Tc2;
  double cacheSizeTemp;
  //double pInTemp;
  long k, iter;
  int numMaxIter = 20;
  
  bool climax;

  float *cumSumRates = new float[catCard];
  cumSumRates[0] = reqRates[colIndex][0];
  for (long k=1; k<catCard; k++)
	  cumSumRates[k] = cumSumRates[k-1] + reqRates[colIndex][k];


  //printf("cSizeTarg: %f\n", cSizeTarg);
  //printf("alphaVal: %f\n", alphaVal);
  //printf("catCard: %d\n", catCard);
  //printf("colIndex: %d\n", colIndex);

  //double lambdaTot = sumRates;
  double lambdaTot = 0;
  for(k=0; k < catCard; k++)
  {
	  lambdaTot += reqRates[colIndex][k];
  }

  //printf("Compute Tc step - 1 -\n");
  Tc = 0.3 * (cSizeTarg/lambdaTot); // Starting value for the Tc
  
  // Iterative procedure to calculate the 'characteristic time' (Tc) using the target cache size.
  iter=0;
  do {
     cacheSizeTemp = 0.0;
     climax = false;
     for (k=0; k<catCard; k++)
     {
    	 if(!climax)			// The descending order of the request rates is not started yet.
    	 {
			 if(strcmp(dp,"LCE") == 0)
			 {
				 if(reqRates[colIndex][k]*Tc <= 0.01)
					 cacheSizeTemp += reqRates[colIndex][k]*Tc;
				 else
				 {
					 climax = true;		// Starting point of the ascending order
					 cacheSizeTemp += 1.0 - exp(-reqRates[colIndex][k]*Tc);
				 }
			 }
			 else if (strcmp(dp, "fixP") == 0)
			 {
				 if(q*reqRates[colIndex][k]*Tc <= 0.01)
					 cacheSizeTemp += q*(reqRates[colIndex][k])*Tc;
				 else
				 {
					 cacheSizeTemp += (q * (1.0 - exp(-reqRates[colIndex][k]*Tc)))/(exp(-reqRates[colIndex][k]*Tc) + q * (1.0 - exp(-reqRates[colIndex][k]*Tc)));
					 climax = true;
				 }
			 }
			 else
			 {
				 printf("Meta Caching Policy NOT SUPPORTED!");
				 exit(0);
			 }
    	 }
    	 else     // We are in the descending order, so we can approximate by using the cdf of the request rates.
    	 {
    		 if(strcmp(dp,"LCE") == 0)
    		 {
    			 cacheSizeTemp += 1.0 - exp(-reqRates[colIndex][k]*Tc);
    			 if(reqRates[colIndex][k]*Tc <= 0.01)
    			 {
    				 cacheSizeTemp += (lambdaTot-cumSumRates[k])*Tc;
    				 break;
    			 }

    		 }
    		 else if (strcmp(dp, "fixP") == 0)
    		 {
    			 cacheSizeTemp += (q * (1.0 - exp(-reqRates[colIndex][k]*Tc)))/(exp(-reqRates[colIndex][k]*Tc) + q * (1.0 - exp(-reqRates[colIndex][k]*Tc)));
    			 if(q*reqRates[colIndex][k]*Tc <= 0.01)
    			 {
    				 cacheSizeTemp += q*(lambdaTot-cumSumRates[k])*Tc;
    				 break;
    			 }
    		 }
    		 else
    		 {
    			 printf("Meta Caching Policy NOT SUPPORTED!");
    		     exit(0);
    		 }
//    		 if(reqRates[colIndex][k]*Tc <= 0.01)
//    		 {
//    			 //printf("NODE # %d, Approximation since content # %d\n", colIndex, k);
//    		     /*for (long z = k+1 ; z<catCard; z++)
//    		     {
//    		     	cacheSizeTemp += reqRates[colIndex][z]*Tc;		// Linear Term
//    		     	//cacheSizeTemp += reqRates[colIndex][z]*Tc - (pow(reqRates[colIndex][z]*Tc,2)/2);	// Quadratic Term
//    		      }*/
//    			 if(strcmp(dp,"LCE") == 0)
//    				 cacheSizeTemp += (lambdaTot-cumSumRates[k])*Tc;
//    			 else
//    				 cacheSizeTemp += q*(lambdaTot-cumSumRates[k])*Tc;
//    			 break;
//    		 }
    	 }
     }

     Tc = Tc * 2.0;
     iter++;
  } while (cacheSizeTemp < cSizeTarg && (iter < numMaxIter));

  //printf("Cache Size Temp 1: %f\n", cacheSizeTemp);
  //printf("Tc beta: %f\n", Tc);
  // Check for the two extreme cases
  if(iter == 1)
  {
    printf("error Tc too small");
    exit(0);
  }
  if(iter == numMaxIter)
  {
    printf("Error: Tc too large for Node # %d\n", colIndex);
    //exit(0);
    Tc = 5000;
    return Tc;
  }

  Tc2=Tc/2.0;
  Tc1=Tc/4.0;

  do {
     Tc = (Tc1+Tc2)/2.0;
     //printf("*Tc* %f\n", Tc);
     //printf("*Tc1* %f\n", Tc1);
     //printf("*Tc2* %f\n", Tc2);
     cacheSizeTemp = 0.0;
     climax = false;
     for (k=0; k<catCard; k++)
     {
       	 if(!climax)			// The descending order of the request rates is not started yet.
        	 {
    			 if(strcmp(dp,"LCE") == 0)
    			 {
    				 if(reqRates[colIndex][k]*Tc <= 0.01)
    					 cacheSizeTemp += reqRates[colIndex][k]*Tc;
    				 else
    				 {
    					 climax = true;		// Starting point of the ascending order
    					 cacheSizeTemp += 1.0 - exp(-reqRates[colIndex][k]*Tc);
    				 }
    			 }
    			 else if (strcmp(dp, "fixP") == 0)
    			 {
    				 if(q*reqRates[colIndex][k]*Tc <= 0.01)
    					 cacheSizeTemp += q*(reqRates[colIndex][k])*Tc;
    				 else
    				 {
    					 cacheSizeTemp += (q * (1.0 - exp(-reqRates[colIndex][k]*Tc)))/(exp(-reqRates[colIndex][k]*Tc) + q * (1.0 - exp(-reqRates[colIndex][k]*Tc)));
    					 climax = true;
    				 }
    			 }
    			 else
    			 {
    				 printf("Meta Caching Policy NOT SUPPORTED!");
    				 exit(0);
    			 }
        	 }
        	 else     // We are in the descending order, so we can approximate by using the cdf of the request rates.
        	 {
        		 if(strcmp(dp,"LCE") == 0)
        		 {
        			 cacheSizeTemp += 1.0 - exp(-reqRates[colIndex][k]*Tc);
        			 if(reqRates[colIndex][k]*Tc <= 0.01)
        			 {
        				 cacheSizeTemp += (lambdaTot-cumSumRates[k])*Tc;
        				 break;
        			 }

        		 }
        		 else if (strcmp(dp, "fixP") == 0)
        		 {
        			 cacheSizeTemp += (q * (1.0 - exp(-reqRates[colIndex][k]*Tc)))/(exp(-reqRates[colIndex][k]*Tc) + q * (1.0 - exp(-reqRates[colIndex][k]*Tc)));
        			 if(q*reqRates[colIndex][k]*Tc <= 0.01)
        			 {
        				 cacheSizeTemp += q*(lambdaTot-cumSumRates[k])*Tc;
        				 break;
        			 }
        		 }
        		 else
        		 {
        			 printf("Meta Caching Policy NOT SUPPORTED!");
        		     exit(0);
        		 }
    //    		 if(reqRates[colIndex][k]*Tc <= 0.01)
    //    		 {
    //    			 //printf("NODE # %d, Approximation since content # %d\n", colIndex, k);
    //    		     /*for (long z = k+1 ; z<catCard; z++)
    //    		     {
    //    		     	cacheSizeTemp += reqRates[colIndex][z]*Tc;		// Linear Term
    //    		     	//cacheSizeTemp += reqRates[colIndex][z]*Tc - (pow(reqRates[colIndex][z]*Tc,2)/2);	// Quadratic Term
    //    		      }*/
    //    			 if(strcmp(dp,"LCE") == 0)
    //    				 cacheSizeTemp += (lambdaTot-cumSumRates[k])*Tc;
    //    			 else
    //    				 cacheSizeTemp += q*(lambdaTot-cumSumRates[k])*Tc;
    //    			 break;
    //    		 }
        	 }
     }
     if (cacheSizeTemp < cSizeTarg)
        Tc1 = Tc;
     else
        Tc2 = Tc;
    } while (fabs(cacheSizeTemp-cSizeTarg)/cSizeTarg > 0.001);

  	//printf("Cache Size: %f\n", cacheSizeTemp);
  delete cumSumRates;

    return Tc;
}


double compute_Tc_single_Approx_More_Repo(double cSizeTarg, double alphaVal, long catCard, float** reqRates, int colIndex, const char* dp, double q)
{
  double Tc, Tc1, Tc2;
  double cacheSizeTemp;
  //double pInTemp;
  long k, iter;
  int numMaxIter = 20;

  //printf("cSizeTarg: %f\n", cSizeTarg);
  //printf("alphaVal: %f\n", alphaVal);
  //printf("catCard: %d\n", catCard);
  //printf("colIndex: %d\n", colIndex);

  //double lambdaTot = sumRates;
  double lambdaTot = 0;
  for(k=0; k < catCard; k++)
  {
	  lambdaTot += reqRates[colIndex][k];
  }

  //printf("Compute Tc step - 1 -\n");
  Tc = 0.3 * (cSizeTarg/lambdaTot); // Starting value for the Tc

  // Iterative procedure to calculate the 'characteristic time' (Tc) using the target cache size.
  iter=0;
  do {
     cacheSizeTemp = 0.0;
/*     if(strcmp(dp,"LCE") == 0)
     {
    	 for (k=0; k<catCard; k++)
         {
    		 if(reqRates[colIndex][k]*Tc <= 0.01)
				 cacheSizeTemp += reqRates[colIndex][k]*Tc;
			 else
			     cacheSizeTemp += 1.0 - exp(-reqRates[colIndex][k]*Tc);

		 }
     }
	 else if (strcmp(dp, "fixP") == 0)
	 {
		 for (k=0; k<catCard; k++)
	     {
			 if(q*reqRates[colIndex][k]*Tc <= 0.01)
				 cacheSizeTemp += q*(reqRates[colIndex][k])*Tc;
			 else
			 	 cacheSizeTemp += (q * (1.0 - exp(-reqRates[colIndex][k]*Tc)))/(exp(-reqRates[colIndex][k]*Tc) + q * (1.0 - exp(-reqRates[colIndex][k]*Tc)));
		 }
	 }
	 else
	 {
		 printf("Meta Caching Policy NOT SUPPORTED!");
		 exit(0);
	 }
*/

     for (k=0; k<catCard; k++)
     {
    	 if(strcmp(dp,"LCE") == 0)
		 {
			 if(reqRates[colIndex][k]*Tc <= 0.01)
				 cacheSizeTemp += reqRates[colIndex][k]*Tc;
			 else
			 {
				 cacheSizeTemp += 1.0 - exp(-reqRates[colIndex][k]*Tc);
			 }
		 }
		 else if (strcmp(dp, "fixP") == 0)
		 {
			 if(q*reqRates[colIndex][k]*Tc <= 0.01)
				 cacheSizeTemp += q*(reqRates[colIndex][k])*Tc;
			 else
			 {
				 cacheSizeTemp += (q * (1.0 - exp(-reqRates[colIndex][k]*Tc)))/(exp(-reqRates[colIndex][k]*Tc) + q * (1.0 - exp(-reqRates[colIndex][k]*Tc)));
			 }
		 }
		 else
		 {
			 printf("Meta Caching Policy NOT SUPPORTED!");
			 exit(0);
		 }
     }
     Tc = Tc * 2.0;
     iter++;
  } while (cacheSizeTemp < cSizeTarg && (iter < numMaxIter));

  //printf("Cache Size Temp 1: %f\n", cacheSizeTemp);
  //printf("Tc beta: %f\n", Tc);
  // Check for the two extreme cases
  if(iter == 1)
  {
    printf("error Tc too small");
    exit(0);
  }
  if(iter == numMaxIter)
  {
    printf("Error: Tc too large for Node # %d\n", colIndex);
    //exit(0);
    Tc = 100000;
    return Tc;
  }

  Tc2=Tc/2.0;
  Tc1=Tc/4.0;

  do {
     Tc = (Tc1+Tc2)/2.0;
     //printf("*Tc* %f\n", Tc);
     //printf("*Tc1* %f\n", Tc1);
     //printf("*Tc2* %f\n", Tc2);
     cacheSizeTemp = 0.0;
     //climax = false;

/*     if(strcmp(dp,"LCE") == 0)
     {
    	 for (k=0; k<catCard; k++)
         {
    		 if(reqRates[colIndex][k]*Tc <= 0.01)
				 cacheSizeTemp += reqRates[colIndex][k]*Tc;
			 else
			     cacheSizeTemp += 1.0 - exp(-reqRates[colIndex][k]*Tc);

		 }
     }
	 else if (strcmp(dp, "fixP") == 0)
	 {
		 for (k=0; k<catCard; k++)
	     {
			 if(q*reqRates[colIndex][k]*Tc <= 0.01)
				 cacheSizeTemp += q*(reqRates[colIndex][k])*Tc;
			 else
			 	 cacheSizeTemp += (q * (1.0 - exp(-reqRates[colIndex][k]*Tc)))/(exp(-reqRates[colIndex][k]*Tc) + q * (1.0 - exp(-reqRates[colIndex][k]*Tc)));
		 }
	 }
	 else
	 {
		 printf("Meta Caching Policy NOT SUPPORTED!");
		 exit(0);
	 }
*/


     for (k=0; k<catCard; k++)
     {
    	 if(strcmp(dp,"LCE") == 0)
    	 		 {
    	 			 if(reqRates[colIndex][k]*Tc <= 0.01)
    	 				 cacheSizeTemp += reqRates[colIndex][k]*Tc;
    	 			 else
    	 			 {
    	 				 cacheSizeTemp += 1.0 - exp(-reqRates[colIndex][k]*Tc);
    	 			 }
    	 		 }
    	 		 else if (strcmp(dp, "fixP") == 0)
    	 		 {
    	 			 if(q*reqRates[colIndex][k]*Tc <= 0.01)
    	 				 cacheSizeTemp += q*(reqRates[colIndex][k])*Tc;
    	 			 else
    	 			 {
    	 				 cacheSizeTemp += (q * (1.0 - exp(-reqRates[colIndex][k]*Tc)))/(exp(-reqRates[colIndex][k]*Tc) + q * (1.0 - exp(-reqRates[colIndex][k]*Tc)));
    	 			 }
    	 		 }
    	 		 else
    	 		 {
    	 			 printf("Meta Caching Policy NOT SUPPORTED!");
    	 			 exit(0);
    	 		 }
     }

     if (cacheSizeTemp < cSizeTarg)
        Tc1 = Tc;
     else
        Tc2 = Tc;
    } while (fabs(cacheSizeTemp-cSizeTarg)/cSizeTarg > 0.001);

  	//printf("Cache Size: %f\n", cacheSizeTemp);
    return Tc;
}


#ifdef __cplusplus
}
#endif
