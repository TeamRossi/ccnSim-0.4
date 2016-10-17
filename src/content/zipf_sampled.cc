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

/* Special thanks goes to Otmar Ertl, who developed the java code for the 
* generalized Zipf distributed random number generator (available at 
* https://github.com/apache/commons-math/blob/master/src/main/java/org/apache/commons/math4/distribution/ZipfDistribution.java)
* in the context of the Apache Commons Math Library (https://commons.apache.org/proper/commons-math/).
*/

#include "zipf_sampled.h"
#include <iostream>
#include <cmath>

#include "ccnsim.h"
#include "client.h"
#include "statistics.h"
#include <omnetpp.h>

//Register_Class (zipf_sampled);

using namespace std;
#if OMNETPP_VERSION >= 0x0500
    using namespace omnetpp;
#endif

void zipf_sampled::zipf_sampled_initialize()
{
	cout << "Downsize from Zipf:\t" << down << endl;
	cout << "Lambda from Zipf:\t" << lambda << endl;
    	cout <<"Zipf initialization completed"<< endl;
    	return;
}

int zipf_sampled::sample()
{
    while(true)
    {
        //omnetpp::cRNG *rng = omnetpp::getRNG(0);
	//double u = hIntegralNumberOfElements + dblrand(rng) * (hIntegralX1 - hIntegralNumberOfElements);
	//double rd = getEnvir()->getRNG(0)->doubleRand();
	
	// Double RNG compatible with both omnet-4x and omnet-5x
	double rd;
	if(cSimulation::getActiveSimulation()->getContext())
	  rd = cSimulation::getActiveSimulation()->getContext()->getRNG(0)->doubleRand();
	else
	  rd = cSimulation::getActiveEnvir()->getRNG(0)->doubleRand();
	
	double u = hIntegralNumberOfElements + rd * (hIntegralX1 - hIntegralNumberOfElements);
        // u is uniformly distributed in (hIntegralX1, hIntegralNumberOfElements]
        double x = hIntegralInverse(u);
        int k = (int)(x + 0.5);
        // Limit k to the range [1, numberOfElements]
        // (k could be outside due to numerical inaccuracies)
        if (k < 1) {
            k = 1;
        }
        else if (k > numberOfElements) {
            k = numberOfElements;
        }
        // Here, the distribution of k is given by:
        //
        //   P(k = 1) = C * (hIntegral(1.5) - hIntegralX1) = C
        //   P(k = m) = C * (hIntegral(m + 1/2) - hIntegral(m - 1/2)) for m >= 2
        //
        //   where C := 1 / (hIntegralNumberOfElements - hIntegralX1)
        if (k - x <= s || u >= hIntegral(k + 0.5) - h(k))
        {
            // Case k = 1:
            //
            //   The right inequality is always true, because replacing k by 1 gives
            //   u >= hIntegral(1.5) - h(1) = hIntegralX1 and u is taken from
            //   (hIntegralX1, hIntegralNumberOfElements].
            //
            //   Therefore, the acceptance rate for k = 1 is P(accepted | k = 1) = 1
            //   and the probability that 1 is returned as random value is
            //   P(k = 1 and accepted) = P(accepted | k = 1) * P(k = 1) = C = C / 1^exponent
            //
            // Case k >= 2:
            //
            //   The left inequality (k - x <= s) is just a short cut
            //   to avoid the more expensive evaluation of the right inequality
            //   (u >= hIntegral(k + 0.5) - h(k)) in many cases.
            //
            //   If the left inequality is true, the right inequality is also true:
            //     Theorem 2 in the paper is valid for all positive exponents, because
            //     the requirements h'(x) = -exponent/x^(exponent + 1) < 0 and
            //     (-1/hInverse'(x))'' = (1+1/exponent) * x^(1/exponent-1) >= 0
            //     are both fulfilled.
            //     Therefore, f(x) := x - hIntegralInverse(hIntegral(x + 0.5) - h(x))
            //     is a non-decreasing function. If k - x <= s holds,
            //     k - x <= s + f(k) - f(2) is obviously also true which is equivalent to
            //     -x <= -hIntegralInverse(hIntegral(k + 0.5) - h(k)),
            //     -hIntegralInverse(u) <= -hIntegralInverse(hIntegral(k + 0.5) - h(k)),
            //     and finally u >= hIntegral(k + 0.5) - h(k).
            //
            //   Hence, the right inequality determines the acceptance rate:
            //   P(accepted | k = m) = h(m) / (hIntegrated(m+1/2) - hIntegrated(m-1/2))
            //   The probability that m is returned is given by
            //   P(k = m and accepted) = P(accepted | k = m) * P(k = m) = C * h(m) = C / m^exponent.
            //
            // In both cases the probabilities are proportional to the probability mass function
            // of the Zipf distribution.
            return k;
        }
    }
}

double zipf_sampled::hIntegral(double x)
{
    double logX = log(x);
    return helper2((1.0-exponent)*logX)*logX;
}

double zipf_sampled::h(double x)
{
    return exp(-exponent * log(x));
}


double zipf_sampled::hIntegralInverse(double x)
{
    double t = x*(1.0-exponent);
    if (t < -1.0)
    {
        // Limit value to the range [-1, +inf).
        // t could be smaller than -1 in some rare cases due to numerical errors.
        t = -1;
    }
    return exp(helper1(t)*x);
}


double zipf_sampled::helper1(double x)
{
    if (abs(x)>1e-8)
    {
        return log(1.0+x)/x;
    }
    else
    {
        return 1.-x*((1./2.)-x*((1./3.)-x*(1./4.)));
    }
}

double zipf_sampled::helper2(double x)
{
    if (abs(x)>1e-8)
    {
        return (exp(x)-1.0)/x;
    }
    else
    {
        return 1.+x*(1./2.)*(1.+x*(1./3.)*(1.+x*(1./4.)));
    }
}


double zipf_sampled::generalizedHarmonic(int n, double m)
{
    double value = 0;
    for (int k = n; k > 0; --k)
    {
        value += 1.0 / pow(k, m);
    }
    return value;
}


double zipf_sampled::probability(int x)
{
    if (x <= 0 || x > numberOfElements)
    {
        return 0.0;
    }

    return (1.0 / pow(x, exponent)) * normalization_constant;
}



double zipf_sampled::cumulativeProbability(int x)
{
    if (x <= 0)
    {
        return 0.0;
    } else if (x >= numberOfElements)
    {
        return 1.0;
    }
    return generalizedHarmonic(x, exponent) * normalization_constant;
}
