#ifndef STANDALONE
#include "include/base/Parameter.h"
#include "include/ROC/ROCParameter.h"
#include "include/RFP/RFPParameter.h"
#include "include/FONSE/FONSEParameter.h"
#include <Rcpp.h>
using namespace Rcpp;

RCPP_EXPOSED_CLASS(ROCTrace)
RCPP_EXPOSED_CLASS(RFPTrace)
RCPP_EXPOSED_CLASS(FONSETrace)
RCPP_EXPOSED_CLASS(Genome)
RCPP_EXPOSED_CLASS(CovarianceMatrix)




RCPP_MODULE(Parameter_mod)
{
	class_<Parameter>(	"Parameter" )
		.method("readPhiValues", &Parameter::readPhiValues)
		.method("setMixtureAssignmentForGene", &Parameter::setMixtureAssignmentForGene)
		.method("getMutationCategoryForMixture", &Parameter::getMutationCategoryForMixture)
		.method("getSelectionCategoryForMixture", &Parameter::getSelectionCategoryForMixture)
		.method("getSynthesisRateCategoryForMixture", &Parameter::getSynthesisRateCategoryForMixture)

		.method("initializeSynthesisRateByGenome", &Parameter::initializeSynthesisRateByGenome)
		.method("initializeSynthesisRateByList", &Parameter::initializeSynthesisRateByList)
		.method("initializeSynthesisRateByRandom", &Parameter::initializeSynthesisRateByRandom)
		.method("getEstimatedMixtureAssignmentForGene", &Parameter::getEstimatedMixtureAssignmentForGene, "returns the mixture assignment for a given gene")
		.method("getEstimatedMixtureAssignmentProbabilitiesForGene", &Parameter::getEstimatedMixtureAssignmentProbabilitiesForGene, "returns the probabilities assignment for a given gene")
		.method("getSynthesisRatePosteriorMeanByMixtureElementForGene", &Parameter::getSynthesisRatePosteriorMeanByMixtureElementForGene)
		.method("getSynthesisRateVarianceByMixtureElementForGene", &Parameter::getSynthesisRateVarianceByMixtureElementForGene)
		.method("getCurrentSynthesisRateForMixture", &Parameter::getCurrentSynthesisRateForMixture)
		.method("getMixtureAssignmentForGene", &Parameter::getMixtureAssignmentForGeneR)
		

		.property("numMutationCategories", &Parameter::getNumMutationCategories)
		.property("numSelectionCategories", &Parameter::getNumSelectionCategories)
		.property("numMixtures", &Parameter::getNumMixtureElements)
		;


	class_<ROCParameter>( "ROCParameter" )
		.derives<Parameter>("Parameter")
		.constructor <std::string>()
		.constructor <double, std::vector<unsigned>, std::vector<unsigned>, bool>()
		.constructor <double, unsigned, std::vector<unsigned>, bool, std::string>()
		.method("initSelection", &ROCParameter::initSelection)
		.method("initMutation", &ROCParameter::initMutation)
		.method("getTraceObject", &ROCParameter::getTraceObject)
		.method("initCovarianceMatrix", &ROCParameter::initCovarianceMatrix)
		.method("getCovarianceMatrixForAA", &ROCParameter::getCovarianceMatrixForAA)
		.method("initMutationCategories", &ROCParameter::initMutationCategories)
		.method("initSelectionCategories", &ROCParameter::initSelectionCategories)

		.method("calculateSelectionCoefficients", &ROCParameter::calculateSelectionCoefficientsR)
		

		.method("getSphiPosteriorMean", &ROCParameter::getSphiPosteriorMean)

		.method("getMutationPosteriorMeanForCodon", &ROCParameter::getMutationPosteriorMeanForCodon)
		.method("getSelectionPosteriorMeanForCodon", &ROCParameter::getSelectionPosteriorMeanForCodon)

		.method("getSphiVariance", &ROCParameter::getSphiVariance)
		.method("getAphiVariance", &ROCParameter::getAphiVariance)

		.method("getMutationPriorStandardDeviation", &ROCParameter::getMutationPriorStandardDeviation)
		.method("setMutationPriorStandardDeviation", &ROCParameter::setMutationPriorStandardDeviation)

		.method("getMutationVarianceForCodon", &ROCParameter::getMutationVarianceForCodon)
		.method("getSelectionVarianceForCodon", &ROCParameter::getSelectionVarianceForCodon)

		;

	class_<RFPParameter>( "RFPParameter" )
		.derives<Parameter>("Parameter")
		.constructor <std::string>()
		.constructor <double, std::vector<unsigned>, std::vector<unsigned>, bool>()
		.constructor <double, unsigned, std::vector<unsigned>, bool, std::string>()

		.method("initAlpha", &RFPParameter::initAlphaR)
		.method("initLambdaPrime", &RFPParameter::initLambdaPrimeR)
		.method("getTraceObject", &RFPParameter::getTraceObject)
		.method("getParameterForCategory", &RFPParameter::getParameterForCategoryR)
		.method("initMutationSelectionCategories", &RFPParameter::initMutationSelectionCategoriesR)
		.method("getAlphaPosteriorMeanForCodon", &RFPParameter::getAlphaPosteriorMeanForCodon)
		.method("getLambdaPrimePosteriorMeanForCodon", &RFPParameter::getLambdaPrimePosteriorMeanForCodon)
		.method("getAlphaVarianceForCodon", &RFPParameter::getAlphaVarianceForCodon)
		.method("getLambdaPrimeVarianceForCodon", &RFPParameter::getLambdaPrimeVarianceForCodon)
		.method("getTmpTrace", &RFPParameter::getTmp)
		;

	class_<FONSEParameter>("FONSEParameter")
		.derives<Parameter>("Parameter")
		.constructor <std::string>()
		.constructor <double, std::vector<unsigned>, std::vector<unsigned>, bool>()
		.constructor <double, unsigned, std::vector<unsigned>, bool, std::string>()
		.method("initMutationSelectionCategories", &FONSEParameter::initMutationSelectionCategoriesR)
		.method("initSelection", &FONSEParameter::initSelection)
		.method("initMutation", &FONSEParameter::initMutation)
		.method("getTraceObject", &FONSEParameter::getTraceObject)

		//R wrapper functions
		.method("calculateSelectionCoefficients", &FONSEParameter::calculateSelectionCoefficientsR)


		// Posterior functions
		.method("getSphiPosteriorMean", &FONSEParameter::getSphiPosteriorMean)

		//R wrapper functions
		.method("getMutationPosteriorMeanForCodon", &FONSEParameter::getMutationPosteriorMeanForCodon)
		.method("getSelectionPosteriorMeanForCodon", &FONSEParameter::getSelectionPosteriorMeanForCodon)

		// Variance functions
		.method("getSphiVariance", &FONSEParameter::getSphiVariance)

		//R wrapper functions
		.method("getMutationVarianceForCodon", &FONSEParameter::getMutationVarianceForCodon)
		.method("getSelectionVarianceForCodon", &FONSEParameter::getSelectionVarianceForCodon)
		.method("initCovarianceMatrix", &FONSEParameter::initCovarianceMatrix)
		.method("getCovarianceMatrixForAA", &FONSEParameter::getCovarianceMatrixForAA)
		;
}
#endif
