// FONSEModel.h

#ifndef FONSEMODEL_H
#define FONSEMODEL_H

#include "../base/Model.h"
#include "FONSEParameter.h"

class FONSEModel : public Model
{
private:

	FONSEParameter *parameter;
	double calculateLogLikelihoodPerPositionPerGene(unsigned position, unsigned numCodons, double phiValue, double codonProb[]);

public:

	FONSEModel();
	virtual ~FONSEModel();
	//FONSEModel(const FONSEModel& other);


	virtual void simulateGenome(Genome &genome) {} //TODO: needs implimentation
	void setParameter(FONSEParameter &_parameter);
	void calculateCodonProbabilityVector(unsigned numCodons, unsigned position, unsigned maxIndexValue, double* mutation, double* selection, double phi, double codonProb[]);
	// Likelihood ratio functions
	virtual void calculateLogLikelihoodRatioPerGene(Gene& gene, unsigned geneIndex, unsigned k, double* logProbabilityRatio);
	double calculateLogLikelihoodRatioPerAA(Gene& gene, std::string grouping, double *mutation, double *selection, double phiValue);
	virtual void calculateLogLikelihoodRatioPerGroupingPerCategory(std::string grouping, Genome& genome, double& logAcceptanceRatioForAllMixtures);

	//Parameter wrapper functions:
	virtual void initTraces(unsigned samples, unsigned num_genes) { parameter->initAllTraces(samples, num_genes); }
	virtual void writeRestartFile(std::string filename) { return parameter->writeEntireRestartFile(filename); }
	virtual double getSphi(bool proposed = false) { return parameter->getSphi(proposed); }
	virtual double getSphiProposalWidth() { return parameter->getSphiProposalWidth(); }
	virtual unsigned getNumMixtureElements() { return parameter->getNumMixtureElements(); }
	virtual double getCategoryProbability(unsigned i) { return parameter->getCategoryProbability(i); }
	virtual void proposeCodonSpecificParameter() { parameter->proposeCodonSpecificParameter(); }
	virtual void adaptCodonSpecificParameterProposalWidth(unsigned adaptiveWidth) { parameter->adaptCodonSpecificParameterProposalWidth(adaptiveWidth); }
	virtual void updateCodonSpecificParameter(std::string grouping) { parameter->updateCodonSpecificParameter(grouping); }
	virtual void updateCodonSpecificParameterTrace(unsigned sample, std::string grouping) { parameter->updateCodonSpecificParameterTrace(sample, grouping); }
	virtual void proposeSPhi() { parameter->proposeSPhi(); }
	virtual unsigned getMixtureAssignment(unsigned index) { return parameter->getMixtureAssignment(index); }
	virtual unsigned getSynthesisRateCategory(unsigned mixture) { return parameter->getSynthesisRateCategory(mixture); }
	virtual unsigned getSelectionCategory(unsigned mixture) { return parameter->getSelectionCategory(mixture); }
	virtual unsigned getMutationCategory(unsigned mixture)  { return parameter->getMutationCategory(mixture); }
	virtual double getSynthesisRate(unsigned index, unsigned mixture, bool proposed = false) { return parameter->getSynthesisRate(index, mixture, proposed); }
	virtual double getCurrentSphiProposalWidth() { return parameter->getCurrentSphiProposalWidth(); }
	virtual void updateSphi() { parameter->updateSphi(); }
	virtual void updateSphiTrace(unsigned sample) { parameter->updateSphiTrace(sample); }
	virtual void adaptSphiProposalWidth(unsigned adaptiveWidth) { parameter->adaptSphiProposalWidth(adaptiveWidth); }
	virtual void proposeSynthesisRateLevels() { parameter->proposeSynthesisRateLevels(); }
	virtual unsigned getNumSynthesisRateCategories() { return parameter->getNumSynthesisRateCategories(); }
	virtual std::vector<unsigned> getMixtureElementsOfSelectionCategory(unsigned k) { return parameter->getMixtureElementsOfSelectionCategory(k); }
	virtual void updateSynthesisRate(unsigned i, unsigned k) { parameter->updateSynthesisRate(i, k); }
	virtual void setMixtureAssignment(unsigned i, unsigned catOfGene) { parameter->setMixtureAssignment(i, catOfGene); }
	virtual void updateSynthesisRateTrace(unsigned sample, unsigned i) { parameter->updateSynthesisRateTrace(sample, i); }
	virtual void updateMixtureAssignmentTrace(unsigned sample, unsigned i) { parameter->updateMixtureAssignmentTrace(sample, i); }
	virtual void setCategoryProbability(unsigned mixture, double value) { parameter->setCategoryProbability(mixture, value); }
	virtual void updateMixtureProbabilitiesTrace(unsigned sample) { parameter->updateMixtureProbabilitiesTrace(sample); }
	virtual void adaptSynthesisRateProposalWidth(unsigned adaptiveWidth) { parameter->adaptSynthesisRateProposalWidth(adaptiveWidth); }
	virtual void getParameterForCategory(unsigned category, unsigned param, std::string aa, bool proposal, double* returnValue)
	{
		parameter->getParameterForCategory(category, param, aa, proposal, returnValue);
	}
	virtual unsigned getGroupListSize() { return parameter->getGroupListSize(); } //TODO: make not hardcoded?
	virtual std::string getGrouping(unsigned index) { return parameter->getGrouping(index); }
	// R wrapper
	//std::vector<double> CalculateProbabilitiesForCodons(std::vector<double> mutation, std::vector<double> selection, double phi);

protected:
};

#endif // FONSEMODEL_H
