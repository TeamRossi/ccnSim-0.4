/*
 * ccnSim is a scalable chunk-level simulator for Content Centric
 * Networks (CCN), that we developed in the context of ANR Connect
 * (http://www.anr-connect.org/)
 *
 * People:
 *    Michele Tortelli (Principal suspect 1.0, mailto michele.tortelli@telecom-paristech.fr)
 *    Andrea Araldo (Principal suspect 1.1, mailto araldo@lri.fr)
 *    Dario Rossi (Occasional debugger, mailto dario.rossi@enst.fr)
 *    Emilio Leonardi (Well informed outsider, mailto emilio.leonardi@tlc.polito.it)
 *    Giuseppe Rossini (Former lead developer, mailto giuseppe.rossini@enst.fr)
 *    Raffaele Chiocchetti (Former developer, mailto raffaele.chiocchetti@gmail.com)
 *
 * Mailing list: 
 *    ccnsim@listes.telecom-paristech.fr
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ZIPF_SAMPLED_H_
#define ZIPF_SAMPLED_H_
#include <omnetpp.h>
#include <vector>
#include "error_handling.h"
#if OMNETPP_VERSION < 0x0500
    #include <csimplemodule.h>
#else
    #include <omnetpp/csimplemodule.h>
#endif

using namespace std;

/**
* 
* Special thanks goes to Otmar Ertl, who developed the java code for the 
* generalized Zipf distributed random number generator (available at 
* https://github.com/apache/commons-math/blob/master/src/main/java/org/apache/commons/math4/distribution/ZipfDistribution.java)
* in the context of the Apache Commons Math Library (https://commons.apache.org/proper/commons-math/).
*
*
* Utility class implementing a rejection inversion sampling method for a discrete,
* bounded Zipf distribution that is based on the method described in
* Wolfgang Hörmann and Gerhard Derflinger
* "Rejection-inversion to generate variates from monotone discrete distributions."
* ACM Transactions on Modeling and Computer Simulation (TOMACS) 6.3 (1996): 169-184.
* The paper describes an algorithm for exponents larger than 1 (Algorithm ZRI).
* The original method uses H(x) := (v + x)^(1 - q) / (1 - q)
* as the integral of the hat function. This function is undefined for
* q = 1, which is the reason for the limitation of the exponent.
* If instead the integral function
* H(x) := ((v + x)^(1 - q) - 1) / (1 - q) is used,
* for which a meaningful limit exists for q = 1,
* the method works for all positive exponents.
* The following implementation uses v := 0 and generates integral numbers
* in the range [1, numberOfElements]. This is different to the original method
* where v is defined to be positive and numbers are taken from [0, i_max].
* This explains why the implementation looks slightly different.
*/
class zipf_sampled {

    public:
    //private:
        int numberOfElements;               // Number of elements.
        double exponent;                    // Exponent parameter of the distribution.
        double hIntegralX1;                 // Constant equal to {hIntegral(1.5) - 1}
        double hIntegralNumberOfElements;   // Constant equal to {hIntegral(numberOfElements + 0.5)}.
        double s;                           // Constant equal to {2 - hIntegralInverse(hIntegral(2.5) - h(2)}.

        double normalization_constant;      // a.k.a n-th harmonic
        double lambda;
        int down;							// Downsizing factor;
        int newCard;						// Downsized cardinality;

        void zipf_sampled_initialize();
	

    //public:

        /** Simple constructor.
         * @param numOfElem number of elements
         * @param alpha exponent parameter of the distribution
         */
        zipf_sampled(int newCardinality, double alpha, double rate, int downF){
            exponent = alpha;
            down = downF;

            if(downF > 1)						// TTl-based scenario
            	numberOfElements = downF;
            else if(downF == 1)					// ED-sim
            	numberOfElements = newCardinality;
            else
            {
            	std::stringstream ermsg;
            	ermsg<<"Please insert a downsizing factor Delta >= 1.";
            	severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
            }

            newCard = newCardinality;
            lambda = rate;
            hIntegralX1 = hIntegral(1.5) - 1.0;
            hIntegralNumberOfElements = hIntegral(numberOfElements + 0.5);
            s = 2.0 - hIntegralInverse(hIntegral(2.5) - h(2));
            normalization_constant = generalizedHarmonic(newCard*down, exponent);
            normalization_constant  = 1.0*(1./normalization_constant);
        }
        zipf_sampled(int numOfElem, double alpha):numberOfElements(numOfElem),exponent(alpha){;};
        zipf_sampled(){zipf_sampled(0,0.0);}


        /**
        * Calculates the Nth generalized harmonic number.
        *
        * @param n Term in the series to calculate (must be larger than 1)
        * @param m Exponent (special case (m = 1) is the harmonic series).
        * @return the n^th generalized harmonic number.
        */
        double generalizedHarmonic(int content, double alpha);


        /** Generate one integral number in the range [1, numberOfElements].
         * @param random random generator to use
         * @return generated integral number in the range [1, numberOfElements]
         */
        int sample();


        /**
         * H(x) = (x^(1-exponent) - 1)/(1 - exponent), if exponent!=1
                = log(x), if exponent==1
         *
         * H(x) is an integral function of h(x),
         * the derivative of H(x) is h(x).
         *
         * @param x free parameter
         * @return H(x)
         */
        double hIntegral(double x);

        /**
         * h(x) = 1/x^exponent
         *             * @param x free parameter
         * @return h(x)
         */
        double h(double x);

        /**
         * The inverse function of H(x).
         *
         * @param x free parameter
         * @return y for which H(y) = x
         */
        double hIntegralInverse(double x);

        /**
         * Helper function that calculates log(1+x)/x.
         *
         * A Taylor series expansion is used, if x is close to 0.
         *
         * @param x a value larger than or equal to -1
         * @return log(1+x)/x
         */
        double helper1(double x);

        /**
         * Helper function to calculate (exp(x)-1)/x.
         *
         * A Taylor series expansion is used, if x is close to 0.
         *
         * @param x free parameter
         * @return (exp(x)-1)/x if x is non-zero, or 1 if x=0
         */
        double helper2(double x);

        double probability(int x);
        double cumulativeProbability(int x);

        double get_normalization_constant(){return normalization_constant;}
        unsigned int get_catalog_card(){return down*newCard;}
        double get_alpha(){return exponent;}
};

#endif
