
#include "ReplicationError.hpp"
#include <stdio.h>

using namespace QuantLib;
using namespace std;


void ReplicationError::compute(Size nTimeSteps, Size nSamples)
{
    QL_REQUIRE(nTimeSteps>0, "the number of steps must be > 0");
    
    // if interval set,
    // Time tau = maturity_ / nTimeSteps;
    
    Calendar calendar = SouthKorea();
    Date today = Date::todaysDate();
    DayCounter dayCount = Actual365Fixed();
    Handle<Quote> stateVariable(ext::shared_ptr<Quote>(new SimpleQuote(s0_)));
    Handle<YieldTermStructure> riskFreeRate(ext::shared_ptr<YieldTermStructure>(new FlatForward(today, r_, dayCount)));
    Handle<YieldTermStructure> dividendYield(ext::shared_ptr<YieldTermStructure>(new FlatForward(today, q_, dayCount)));
    Handle<BlackVolTermStructure> volatility(ext::shared_ptr<BlackVolTermStructure>(new BlackConstantVol(today, calendar, sigma_, dayCount)));
    ext::shared_ptr<StochasticProcess1D> diffusion(new BlackScholesMertonProcess(stateVariable, dividendYield, riskFreeRate, volatility));
    
    //
    
    PseudoRandom::rsg_type rsg = PseudoRandom::make_sequence_generator(nTimeSteps, 1);
    bool brownianBridge = false;
    typedef SingleVariate<PseudoRandom>::path_generator_type generator_type;
    ext::shared_ptr<generator_type> myPathGenerator(new generator_type(diffusion, maturity_, nTimeSteps, rsg, brownianBridge));
    
    ext::shared_ptr<PathPricer<Path> > myPathPricer(new ReplicationPathPricer(payoff_.optionType(), payoff_.strike(),
                                                                              r_, q_, maturity_, sigma_));
    Statistics statisticsAccumulator;
    
    MonteCarloModel<SingleVariate, PseudoRandom> MCSimulation(myPathGenerator, myPathPricer, statisticsAccumulator, false);
    MCSimulation.addSamples(nSamples);
    
    Real PLMean = MCSimulation.sampleAccumulator().mean();
    Real PLStddev = MCSimulation.sampleAccumulator().standardDeviation();
    Real PLSkew = MCSimulation.sampleAccumulator().skewness();
    Real PLKurt = MCSimulation.sampleAccumulator().kurtosis();
    
    // Derman and Kamil //
    Real theorStD = std::sqrt(M_PI/4/nTimeSteps)*vega_*sigma_;
    std::cout << std::fixed
    << std::setprecision(3)
    <<std::setw(8) << nSamples << " | "
    <<std::setw(8) << nTimeSteps << " | "
    <<std::setw(8) << PLMean << " | "
    <<std::setw(8) << PLStddev << " | "
    <<std::setw(12) << theorStD << " | "
    <<std::setw(8) << PLSkew << " | "
    <<std::setw(8) << PLKurt << std::endl;
    
}

Real ReplicationPathPricer::operator()(const Path& path) const
{
    Size n = path.length() - 1;
}
