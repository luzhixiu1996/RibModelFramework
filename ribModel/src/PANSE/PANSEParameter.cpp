#include "../include/PANSE/PANSEParameter.h"


#include <sstream>


#ifndef STANDALONE
#include <Rcpp.h>
using namespace Rcpp;
#endif


PANSEParameter::PANSEParameter() : Parameter()
{
	//ctor
}


PANSEParameter::PANSEParameter(std::string filename) : Parameter(64)
{
	initFromRestartFile(filename);
	numParam = 61;
}


PANSEParameter::PANSEParameter(std::vector<double> sphi, unsigned _numMixtures, std::vector<unsigned> geneAssignment, std::vector<std::vector<unsigned>> thetaKMatrix,
		bool splitSer, std::string _mutationSelectionState) : Parameter(64)
{
	initParameterSet(sphi, numMixtures, geneAssignment, thetaKMatrix, splitSer, _mutationSelectionState);
	initPANSEParameterSet();
}


PANSEParameter::~PANSEParameter()
{
	//dtor 
	//TODO: Need to call Parameter's deconstructor
}


PANSEParameter& PANSEParameter::operator=(const PANSEParameter& rhs)
{
	if (this == &rhs) return *this; // handle self assignment

	Parameter::operator=(rhs);

	currentAlphaParameter = rhs.currentAlphaParameter;
	proposedAlphaParameter = rhs.proposedAlphaParameter;
	currentLambdaPrimeParameter = rhs.proposedLambdaPrimeParameter;
	proposedLambdaPrimeParameter = rhs.proposedLambdaPrimeParameter;

	lambdaValues = rhs.lambdaValues;
	numAcceptForAlphaAndLambdaPrime = rhs.numAcceptForAlphaAndLambdaPrime;

	bias_csp = rhs.bias_csp;
	std_csp = rhs.std_csp;

	return *this;
}


void PANSEParameter::initPANSEParameterSet()
{

	unsigned alphaCategories = getNumMutationCategories();
	unsigned lambdaPrimeCategories = getNumSelectionCategories();

	currentAlphaParameter.resize(alphaCategories);
	proposedAlphaParameter.resize(alphaCategories);
	currentLambdaPrimeParameter.resize(lambdaPrimeCategories);
	proposedLambdaPrimeParameter.resize(lambdaPrimeCategories);
	lambdaValues.resize(lambdaPrimeCategories);
	numParam = 61;

	for (unsigned i = 0; i < alphaCategories; i++)
	{
		std::vector <double> tmp(numParam,1.0);
		currentAlphaParameter[i] = tmp;
		proposedAlphaParameter[i] = tmp;
	}
	for (unsigned i = 0; i < lambdaPrimeCategories; i++)
	{
		std::vector <double> tmp(numParam,1.0);
		currentLambdaPrimeParameter[i] = tmp;
		proposedLambdaPrimeParameter[i] = tmp;
		lambdaValues[i] = tmp; //Maybe we don't initialize this one? or we do it differently?
	}

	numAcceptForAlphaAndLambdaPrime.resize(maxGrouping, 0u);
	bias_csp = 0;
	std_csp.resize(numParam, 0.1);

	groupList = {"GCA", "GCC", "GCG", "GCT", "TGC", "TGT", "GAC", "GAT", "GAA", "GAG",
		"TTC", "TTT", "GGA", "GGC", "GGG", "GGT", "CAC", "CAT", "ATA", "ATC",
		"ATT", "AAA", "AAG", "CTA", "CTC", "CTG", "CTT", "TTA", "TTG", "ATG",
		"AAC", "AAT", "CCA", "CCC", "CCG", "CCT", "CAA", "CAG", "AGA", "AGG",
		"CGA", "CGC", "CGG", "CGT", "TCA", "TCC", "TCG", "TCT", "ACA", "ACC",
		"ACG", "ACT", "GTA", "GTC", "GTG", "GTT", "TGG", "TAC", "TAT", "AGC",
		"AGT"};
}


void PANSEParameter::initAlpha(double alphaValue, unsigned mixtureElement, std::string codon)
{
	unsigned category = getMutationCategory(mixtureElement);
	unsigned index = SequenceSummary::codonToIndex(codon);
	currentAlphaParameter[category][index] = alphaValue;
}


void PANSEParameter::initLambdaPrime(double lambdaPrimeValue, unsigned mixtureElement, std::string codon)
{
	unsigned category = getMutationCategory(mixtureElement);
	unsigned index = SequenceSummary::codonToIndex(codon);
	currentLambdaPrimeParameter[category][index] = lambdaPrimeValue;
}


void PANSEParameter::initMutationSelectionCategories(std::vector<std::string> files, unsigned numCategories, unsigned paramType)
{
	std::ifstream currentFile;
	std::string tmpString;
	std::string type;


	if (paramType == PANSEParameter::alp)
		type = "alpha";
	else
		type = "lambda";


	for (unsigned i = 0; i < numCategories; i++)
	{
		std::vector<double> temp(numParam, 0.0);

		//open the file, make sure it opens
		currentFile.open(files[i].c_str());
		if (currentFile.fail())
		{
			std::cerr << "Error opening file " << i << " in the file vector.\n";
			std::exit(1);
		}
		currentFile >> tmpString; //trash the first line, no info given.

		//expecting CTG,3.239 as the current format
		while (currentFile >> tmpString)
		{
			std::string codon = tmpString.substr(0, 3);
			std::size_t pos = tmpString.find(",", 3);
			std::string val = tmpString.substr(pos + 1, std::string::npos);
			unsigned index = SequenceSummary::codonToIndex(codon, false);
			temp[index] = std::atof(val.c_str());
		}
		unsigned altered = 0u;
		for (unsigned j = 0; j < categories.size(); j++)
		{
			if (paramType == PANSEParameter::alp && categories[j].delM == i)
			{
				currentAlphaParameter[j] = temp;
				proposedAlphaParameter[j] = temp;
				altered++;
			}
			else if (paramType == PANSEParameter::lmPri && categories[j].delEta == i)
			{
				currentLambdaPrimeParameter[j] = temp;
				proposedLambdaPrimeParameter[j] = temp;
				altered++;
			}
			if (altered == numCategories)
				break; //to not access indicies out of bounds.
		}
		currentFile.close();
	}
}


void PANSEParameter::writeEntireRestartFile(std::string filename)
{
	writeBasicRestartFile(filename);
	writePANSERestartFile(filename);
}


void PANSEParameter::writePANSERestartFile(std::string filename)
{

	std::ofstream out;
	std::string output = "";
	std::ostringstream oss;
	unsigned i, j;
	out.open(filename.c_str(), std::ofstream::app);
	if (out.fail())
	{
		std::cerr <<"Could not open restart file for writing\n";
		std::exit(1);
	}

	oss <<">currentAlphaParameter:\n";
	for (i = 0; i < currentAlphaParameter.size(); i++)
	{
		oss << "***\n";
		for (j = 0; j < currentAlphaParameter[i].size(); j++)
		{
			oss << currentAlphaParameter[i][j];
			if ((j + 1) % 10 == 0)
				oss << "\n";
			else
				oss << " ";
		}
		if (j % 10 != 0)
			oss << "\n";
	}

	oss <<">currentLambdaPrimeParameter:\n";
	for (i = 0; i < currentLambdaPrimeParameter.size(); i++)
	{
		oss << "***\n";
		for (j = 0; j < currentLambdaPrimeParameter[i].size(); j++)
		{
			oss << currentLambdaPrimeParameter[i][j];
			if ((j + 1) % 10 == 0)
				oss << "\n";
			else
				oss << " ";
		}
		if (j % 10 != 0)
			oss << "\n";
	}

	oss << ">std_csp:\n";
	std::cout << std_csp.size() <<"\n";
	for (i = 0; i < std_csp.size(); i++)
	{
		oss << std_csp[i];
		if ((i + 1) % 10 == 0)
			oss << "\n";
		else
			oss << " ";
	}
	if (i % 10 != 0)
		oss << "\n";

	output = oss.str();
	out << output;
	out.close();

}


void PANSEParameter::initFromRestartFile(std::string filename)
{
	initBaseValuesFromFile(filename);
	initPANSEValuesFromFile(filename);
}


void PANSEParameter::initPANSEValuesFromFile(std::string filename)
{
	std::ifstream input;
	input.open(filename.c_str());
	if (input.fail())
	{
		std::cerr << "Could not open RestartFile.txt to initialzie ROC values\n";
		std::exit(1);
	}
	std::string tmp, variableName;
	unsigned cat = 0;
	while (getline(input, tmp))
	{
		int flag;
		if (tmp[0] == '>')
			flag = 1;
		else if (input.eof() || tmp == "\n")
			flag = 2;
		else if (tmp[0] == '#')
			flag = 3;
		else
			flag = 4;

		if (flag == 1)
		{
			cat = 0;
			variableName = tmp.substr(1, tmp.size() - 2);
		}
		else if (flag == 2)
		{
			std::cout << "here\n";
		}
		else if (flag == 3) //user comment, continue
		{
			continue;
		}
		else
		{
			std::istringstream iss;
			if (variableName == "currentAlphaParameter")
			{
				if (tmp == "***")
				{
					currentAlphaParameter.resize(currentAlphaParameter.size() + 1);
					cat++;
				}
				else if (tmp == "\n")
					continue;
				else
				{
					double val;
					iss.str(tmp);
					while (iss >> val)
					{
						currentAlphaParameter[cat - 1].push_back(val);
					}
				}
			}
			else if (variableName == "currentLambdaPrimeParameter")
			{
				if (tmp == "***")
				{
					currentLambdaPrimeParameter.resize(currentLambdaPrimeParameter.size() + 1);
					cat++;
				}
				else if (tmp == "\n")
					continue;
				else
				{
					double val;
					iss.str(tmp);
					while (iss >> val)
					{
						currentLambdaPrimeParameter[cat - 1].push_back(val);
					}
				}
			}
			else if (variableName == "std_csp")
			{
				double val;
				iss.str(tmp);
				while (iss >> val)
				{
					std_csp.push_back(val);
				}
			}
		}
	}
	input.close();

	bias_csp = 0;
	numAcceptForAlphaAndLambdaPrime.resize(maxGrouping, 0u);
	proposedAlphaParameter.resize(numMutationCategories);
	proposedLambdaPrimeParameter.resize(numSelectionCategories);
	for (unsigned i = 0; i < numMutationCategories; i++)
	{
		proposedAlphaParameter[i] = currentAlphaParameter[i];
	}
	for (unsigned i = 0; i < numSelectionCategories; i++)
	{
		proposedLambdaPrimeParameter[i] = currentLambdaPrimeParameter[i];
	}
}


PANSETrace& PANSEParameter::getTraceObject()
{
	return traces;
}


void PANSEParameter::initAllTraces(unsigned samples, unsigned num_genes)
{
	traces.initAllTraces(samples, num_genes, numMutationCategories, numSelectionCategories, numParam,
						 numMixtures, categories, (unsigned)groupList.size());
}


void PANSEParameter::updateSphiTrace(unsigned sample)
{
	for(unsigned i = 0u; i < numSelectionCategories; i++)
	{
		traces.updateSphiTrace(sample, Sphi[i], i);
	}
}


void PANSEParameter::updateSynthesisRateTrace(unsigned sample, unsigned geneIndex)
{
	traces.updateSynthesisRateTrace(sample, geneIndex, currentSynthesisRateLevel);
}


void PANSEParameter::updateMixtureAssignmentTrace(unsigned sample, unsigned geneIndex)
{
	traces.updateMixtureAssignmentTrace(sample, geneIndex, mixtureAssignment[geneIndex]);
}


void PANSEParameter::updateMixtureProbabilitiesTrace(unsigned samples)
{
	traces.updateMixtureProbabilitiesTrace(samples, categoryProbabilities);
}


void PANSEParameter::updateCodonSpecificParameterTrace(unsigned sample, std::string codon)
{
	traces.updateCodonSpecificParameterTrace(sample, codon, currentLambdaPrimeParameter, currentAlphaParameter);
}


void PANSEParameter::updateCodonSpecificParameter(std::string grouping)
{
	unsigned i = SequenceSummary::codonToIndex(grouping);
	numAcceptForAlphaAndLambdaPrime[i]++;

	for(unsigned k = 0u; k < numMutationCategories; k++)
	{
		currentAlphaParameter[k][i] = proposedAlphaParameter[k][i];
	}
	for(unsigned k = 0u; k < numSelectionCategories; k++)
	{
		currentLambdaPrimeParameter[k][i] = proposedLambdaPrimeParameter[k][i];
	}
}


double PANSEParameter::getCurrentCodonSpecificProposalWidth(unsigned index)
{
	return std_csp[index];
}


std::vector<std::vector<double>> PANSEParameter::getCurrentAlphaParameter()
{
	return currentAlphaParameter;
}


std::vector<std::vector<double>> PANSEParameter::getCurrentLambdaPrimeParameter()
{
	return currentLambdaPrimeParameter;
}


void PANSEParameter::proposeCodonSpecificParameter()
{
	unsigned numAlpha = (unsigned)currentAlphaParameter[0].size();
	unsigned numLambdaPrime = (unsigned)currentLambdaPrimeParameter[0].size();

	for (unsigned i = 0; i < numMutationCategories; i++)
	{
		for (unsigned j = 0; j < numAlpha; j++)
		{
			proposedAlphaParameter[i][j] = std::exp( randNorm( std::log(currentAlphaParameter[i][j]) , std_csp[j]) );
		}
	}

	for (unsigned i = 0; i < numSelectionCategories; i++)
	{
		for (unsigned j = 0; j < numLambdaPrime; j++)
		{
			proposedLambdaPrimeParameter[i][j] = std::exp( randNorm( std::log(currentLambdaPrimeParameter[i][j]) , std_csp[j]) );
		}
	}
}


void PANSEParameter::adaptCodonSpecificParameterProposalWidth(unsigned adaptationWidth)
{

	std::cout << "acceptance ratio for codon:\n";
	for (unsigned i = 0; i < groupList.size(); i++)
	{
		std::cout << groupList[i] << "\t";

		unsigned codonIndex = SequenceSummary::codonToIndex(groupList[i]);
		double acceptanceLevel = (double)numAcceptForAlphaAndLambdaPrime[codonIndex] / (double)adaptationWidth;
		std::cout << acceptanceLevel << " with std_csp = " << std_csp[i] <<"\n";
		traces.updateCspAcceptanceRatioTrace(codonIndex, acceptanceLevel);
		if (acceptanceLevel < 0.2)
		{
			std_csp[i] *= 0.8;
		}
		if (acceptanceLevel > 0.3)
		{
			std_csp[i] *= 1.2;
		}
		numAcceptForAlphaAndLambdaPrime[codonIndex] = 0u;
	}
	std::cout << "\n";
}


//TODO: The only thing stopping this from moving up to Parameter is the trace stuff. Is there a way around this?
void PANSEParameter::adaptSphiProposalWidth(unsigned adaptationWidth)
{
	double acceptanceLevel = (double)numAcceptForSphi / (double)adaptationWidth;
	traces.updateSphiAcceptanceRatioTrace(acceptanceLevel);
	if(acceptanceLevel < 0.2)
	{
		std_sphi *= 0.8;
	}
	if(acceptanceLevel > 0.3)
	{
		std_sphi *= 1.2;
	}
	numAcceptForSphi = 0u;
}


//TODO: The only thing stopping this from moving up to Parameter is the trace stuff. Is there a way around this?
void PANSEParameter::adaptSynthesisRateProposalWidth(unsigned adaptationWidth)
{
	unsigned acceptanceUnder = 0u;
	unsigned acceptanceOver = 0u;

	for(unsigned cat = 0u; cat < numSelectionCategories; cat ++)
	{
		unsigned numGenes = numAcceptForSynthesisRate[cat].size();
		for(unsigned i = 0; i < numGenes; i++)
		{
			double acceptanceLevel = (double)numAcceptForSynthesisRate[cat][i] / (double)adaptationWidth;
			traces.updateSynthesisRateAcceptanceRatioTrace(cat, i, acceptanceLevel);
			if (acceptanceLevel < 0.225)
			{
				std_phi[cat][i] *= 0.8;
				if (acceptanceLevel < 0.2) acceptanceUnder++;
			}
			if (acceptanceLevel > 0.275)
			{
				std_phi[cat][i] *= 1.2;
				if (acceptanceLevel > 0.3) acceptanceOver++;
			}
			numAcceptForSynthesisRate[cat][i] = 0u;
		}
	}
	std::cout << "acceptance ratio for synthesis rate:\n";
	std::cout << "\t acceptance ratio to low: " << acceptanceUnder << "\n";
	std::cout << "\t acceptance ratio to high: " << acceptanceOver << "\n";
}


//TODO: Traces prevent this from being in the parent class
double PANSEParameter::getSphiPosteriorMean(unsigned samples, unsigned mixture)
{
	double posteriorMean = 0.0;
	unsigned selectionCategory = getSelectionCategoryForMixture(mixture);
	std::vector<double> sPhiTrace = traces.getSphiTrace(selectionCategory);
	unsigned traceLength = (unsigned)sPhiTrace.size();

	if(samples > traceLength)
	{
		std::cerr << "Warning in Parameter::getSphiPosteriorMean throws: Number of anticipated samples (" <<
			samples << ") is greater than the length of the available trace (" << traceLength << ")." << "Whole trace is used for posterior estimate! \n";
		samples = traceLength;
	}
	unsigned start = traceLength - samples;
	for(unsigned i = start; i < traceLength; i++)
	{
		posteriorMean += sPhiTrace[i];
	}
	return posteriorMean / (double)samples;
}


//TODO: Traces prevent this from being in the parent class
double PANSEParameter::getSynthesisRatePosteriorMean(unsigned samples, unsigned geneIndex, unsigned mixtureElement)
{
	unsigned expressionCategory = getSynthesisRateCategory(mixtureElement);
	double posteriorMean = 0.0;
	std::vector<double> synthesisRateTrace = traces.getSynthesisRateTraceByMixtureElementForGene(mixtureElement, geneIndex);
	unsigned traceLength = (unsigned)synthesisRateTrace.size();


	if(samples > traceLength)
	{
		std::cerr << "Warning in Parameter::getSynthesisRatePosteriorMean throws: Number of anticipated samples (" <<
			samples << ") is greater than the length of the available trace (" << traceLength << ")." << "Whole trace is used for posterior estimate! \n";
		samples = traceLength;
	}
	unsigned start = traceLength - samples;
	unsigned category = 0u;
	unsigned usedSamples = 0u;
	std::vector<unsigned> mixtureAssignmentTrace = traces.getMixtureAssignmentTraceForGene(geneIndex);
	for(unsigned i = start; i < traceLength; i++)
	{
		category = mixtureAssignmentTrace[i];
		category = getSynthesisRateCategory(category);
		if(category == expressionCategory)
		{
			posteriorMean += synthesisRateTrace[i];
			usedSamples++;
		}
	}
	// Can return NaN if gene was never in category! But that is Ok.
	return posteriorMean / (double)usedSamples;
}


double PANSEParameter::getAlphaPosteriorMean(unsigned mixtureElement, unsigned samples, std::string &codon)
{
	double posteriorMean = 0.0;
	std::vector<double> alphaParameterTrace = traces.getAlphaParameterTraceByMixtureElementForCodon(mixtureElement, codon);
	unsigned traceLength = (unsigned)alphaParameterTrace.size();

	if(samples > traceLength)
	{
		std::cerr << "Warning in PANSEParameter::getAlphaPosteriorMean throws: Number of anticipated samples (" <<
			samples << ") is greater than the length of the available trace (" << traceLength << ")." << "Whole trace is used for posterior estimate! \n";
		samples = traceLength;
	}
	unsigned start = traceLength - samples;
	for(unsigned i = start; i < traceLength; i++)
	{
		posteriorMean += alphaParameterTrace[i];
	}
	return posteriorMean / (double)samples;
}


double PANSEParameter::getLambdaPrimePosteriorMean(unsigned mixtureElement, unsigned samples, std::string &codon)
{
	double posteriorMean = 0.0;
	std::vector<double> lambdaPrimeParameterTrace = traces.getLambdaPrimeParameterTraceByMixtureElementForCodon(mixtureElement, codon);
	unsigned traceLength = (unsigned)lambdaPrimeParameterTrace.size();

	if(samples > traceLength)
	{
		std::cerr << "Warning in PANSEParameter::getLambdaPrimePosteriorMean throws: Number of anticipated samples (" <<
			samples << ") is greater than the length of the available trace (" << traceLength << ")." << "Whole trace is used for posterior estimate! \n";
		samples = traceLength;
	}
	unsigned start = traceLength - samples;
	for(unsigned i = start; i < traceLength; i++)
	{
		posteriorMean += lambdaPrimeParameterTrace[i];
	}
	return posteriorMean / (double)samples;
}


//TODO: Traces prevent this from being in the parent class
double PANSEParameter::getSphiVariance(unsigned samples, unsigned mixture, bool unbiased)
{
	unsigned selectionCategory = getSelectionCategoryForMixture(mixture);
	std::vector<double> sPhiTrace = traces.getSphiTrace(selectionCategory);
	unsigned traceLength = (unsigned)sPhiTrace.size();
	if(samples > traceLength)
	{
		std::cerr << "Warning in Parameter::getSphiVariance throws: Number of anticipated samples (" <<
			samples << ") is greater than the length of the available trace (" << traceLength << ")." << "Whole trace is used for posterior estimate! \n";
		samples = traceLength;
	}
	// has to be mixture since getSphiPosteriorMean does its own mapping
	double posteriorMean = getSphiPosteriorMean(samples, mixture);

	double posteriorVariance = 0.0;

	unsigned start = traceLength - samples;
	for(unsigned i = start; i < traceLength; i++)
	{
		double difference = sPhiTrace[i] - posteriorMean;
		posteriorVariance += difference * difference;
	}
	double normalizationTerm = unbiased ? (1/((double)samples-1.0)) : (1/(double)samples);
	return normalizationTerm * posteriorVariance;
}


//TODO: Traces prevent this from being in the parent class
double PANSEParameter::getSynthesisRateVariance(unsigned samples, unsigned geneIndex, unsigned mixtureElement, bool unbiased)
{
	std::vector<double> synthesisRateTrace = traces.getSynthesisRateTraceByMixtureElementForGene(mixtureElement, geneIndex);
	unsigned traceLength = (unsigned)synthesisRateTrace.size();
	if(samples > traceLength)
	{
		std::cerr << "Warning in Parameter::getSynthesisRateVariance throws: Number of anticipated samples (" <<
			samples << ") is greater than the length of the available trace (" << traceLength << ")." << "Whole trace is used for posterior estimate! \n";
		samples = traceLength;
	}

	double posteriorMean = getSynthesisRatePosteriorMean(samples, geneIndex, mixtureElement);

	double posteriorVariance = 0.0;
	if(!std::isnan(posteriorMean))
	{
		unsigned start = traceLength - samples;
		unsigned category = 0u;
		double difference = 0.0;
		std::vector<unsigned> mixtureAssignmentTrace = traces.getMixtureAssignmentTraceForGene(geneIndex);
		for(unsigned i = start; i < traceLength; i++)
		{
			category = mixtureAssignmentTrace[i];
			category = getSynthesisRateCategory(category);
			difference = synthesisRateTrace[i] - posteriorMean;
			posteriorVariance += difference * difference;
		}
	}
	double normalizationTerm = unbiased ? (1/((double)samples-1.0)) : (1/(double)samples);
	return normalizationTerm * posteriorVariance;
}


double PANSEParameter::getAlphaVariance(unsigned mixtureElement, unsigned samples, std::string &codon, bool unbiased)
{
	std::vector<double> alphaParameterTrace = traces.getAlphaParameterTraceByMixtureElementForCodon(mixtureElement, codon);
	unsigned traceLength = (unsigned)alphaParameterTrace.size();
	if(samples > traceLength)
	{
		std::cerr << "Warning in PANSEParameter::getAlphaVariance throws: Number of anticipated samples (" <<
			samples << ") is greater than the length of the available trace (" << traceLength << ")." << "Whole trace is used for posterior estimate! \n";
		samples = traceLength;
	}

	double posteriorMean = getAlphaPosteriorMean(mixtureElement, samples, codon);

	double posteriorVariance = 0.0;

	unsigned start = traceLength - samples;
	double difference = 0.0;
	for(unsigned i = start; i < traceLength; i++)
	{
		difference = alphaParameterTrace[i] - posteriorMean;
		posteriorVariance += difference * difference;
	}
	double normalizationTerm = unbiased ? (1/((double)samples-1.0)) : (1/(double)samples);
	return normalizationTerm * posteriorVariance;
}


double PANSEParameter::getLambdaPrimeVariance(unsigned mixtureElement, unsigned samples, std::string &codon, bool unbiased)
{
	std::vector<double> lambdaPrimeParameterTrace = traces.getLambdaPrimeParameterTraceByMixtureElementForCodon(mixtureElement, codon);
	unsigned traceLength = (unsigned)lambdaPrimeParameterTrace.size();
	if(samples > traceLength)
	{
		std::cerr << "Warning in PANSEParameter::getSelectionVariance throws: Number of anticipated samples (" <<
			samples << ") is greater than the length of the available trace (" << traceLength << ")." << "Whole trace is used for posterior estimate! \n";
		samples = traceLength;
	}

	double posteriorMean = getLambdaPrimePosteriorMean(mixtureElement, samples, codon);

	double posteriorVariance = 0.0;

	unsigned start = traceLength - samples;
	double difference = 0.0;
	for(unsigned i = start; i < traceLength; i++)
	{
		difference = lambdaPrimeParameterTrace[i] - posteriorMean;
		posteriorVariance += difference * difference;
	}
	double normalizationTerm = unbiased ? (1/((double)samples-1.0)) : (1/(double)samples);
	return normalizationTerm * posteriorVariance;
}


double PANSEParameter::getParameterForCategory(unsigned category, unsigned paramType, std::string codon, bool proposal)
{
	double rv;
	unsigned codonIndex = SequenceSummary::codonToIndex(codon);
	if (paramType == PANSEParameter::alp)
	{
		rv = (proposal ? proposedAlphaParameter[category][codonIndex] : currentAlphaParameter[category][codonIndex]);
	}
	else if (paramType == PANSEParameter::lmPri)
	{
		rv = (proposal ? proposedLambdaPrimeParameter[category][codonIndex] : currentLambdaPrimeParameter[category][codonIndex]);
	}
	else
	{
		std::cerr << "Warning in PANSEParameter::getParameterForCategory: Unkown parameter type: " << paramType << "\n";
		std::cerr << "\tReturning alpha parameter! \n";
		rv = (proposal ? proposedAlphaParameter[category][codonIndex] : currentAlphaParameter[category][codonIndex]);
	}

	return rv;
}


//TODO: Use of Trace prevents this from being in the base class
std::vector <double> PANSEParameter::getEstimatedMixtureAssignmentProbabilities(unsigned samples, unsigned geneIndex)
{
	std::vector<unsigned> mixtureAssignmentTrace = traces.getMixtureAssignmentTraceForGene(geneIndex);
	std::vector<double> probabilities(numMixtures, 0.0);
	unsigned traceLength = (unsigned)mixtureAssignmentTrace.size();

	if (samples > traceLength)
	{
		std::cerr << "Warning in PANSEParameter::getMixtureAssignmentPosteriorMean throws: Number of anticipated samples (" <<
			samples << ") is greater than the length of the available trace (" << traceLength << ")." << "Whole trace is used for posterior estimate! \n";
		samples = traceLength;
	}

	unsigned start = traceLength - samples;
	for (unsigned i = start; i < traceLength; i++)
	{
		unsigned value = mixtureAssignmentTrace[i];
		probabilities[value]++;
	}

	for (unsigned i = 0; i < numMixtures; i++)
	{
		probabilities[i] /= (double)samples;
	}
	return probabilities;
}


//---------------------STATIC VARIABLE DECLARATIONS---------------------//

const unsigned PANSEParameter::alp = 0u;
const unsigned PANSEParameter::lmPri = 1u;


//---------------------R WRAPPER FUNCTIONS---------------------//

void PANSEParameter::initAlphaR(double alphaValue, unsigned mixtureElement, std::string codon)
{
	bool check = checkIndex(mixtureElement, 1, numMixtures);
	if (check)
	{
		mixtureElement--;
		codon[0] = (char)std::toupper(codon[0]);
		codon[1] = (char)std::toupper(codon[1]);
		codon[2] = (char)std::toupper(codon[2]);

		initAlpha(alphaValue, mixtureElement, codon);
	}
}


void PANSEParameter::initLambdaPrimeR(double lambdaPrimeValue, unsigned mixtureElement, std::string codon)
{
	bool check = checkIndex(mixtureElement, 1, numMixtures);
	if (check)
	{
		mixtureElement--;
		codon[0] = (char)std::toupper(codon[0]);
		codon[1] = (char)std::toupper(codon[1]);
		codon[2] = (char)std::toupper(codon[2]);

		initLambdaPrime(lambdaPrimeValue, mixtureElement, codon);
	}
}


double PANSEParameter::getParameterForCategoryR(unsigned mixtureElement, unsigned paramType, std::string codon, bool proposal)
{
	double rv = 0.0;
	bool check = checkIndex(mixtureElement, 1, numMixtures);
	if (check)
	{
		mixtureElement--;
		unsigned category = 0;
		codon[0] = (char)std::toupper(codon[0]);
		codon[1] = (char)std::toupper(codon[1]);
		codon[2] = (char)std::toupper(codon[2]);
		if (paramType == PANSEParameter::alp)
		{
			//THIS NEEDS TO CHANGE!!!!
			category = getMutationCategory(mixtureElement); //really alpha here
		}
		else if (paramType == PANSEParameter::lmPri)
		{
			category = getSelectionCategory(mixtureElement);
		}
		rv = getParameterForCategory(category, paramType, codon, proposal);
	}
	return rv;
}


void PANSEParameter::initMutationSelectionCategoriesR(std::vector<std::string> files, unsigned numCategories,
													std::string paramType)
{
	unsigned value = 0;
	bool check = true;
	if (paramType == "Alpha")
	{
		value = PANSEParameter::alp;
	}
	else if (paramType == "LambdaPrime")
	{
		value = PANSEParameter::lmPri;
	}
	else
	{
		std::cerr << "Bad paramType given. Expected \"Alpha\" or \"LambdaPrime\".\nFunction not being executed!\n";
		check = false;
	}
	if (files.size() != numCategories) //we have different sizes and need to stop
	{
		std::cerr
		<< "The number of files given and the number of categories given differ. Function will not be executed!\n";
		check = false;
	}

	if (check)
	{
		initMutationSelectionCategories(files, numCategories, value);
	}
}


double PANSEParameter::getAlphaPosteriorMeanForCodon(unsigned mixtureElement, unsigned samples, std::string codon)
{
	double rv = -1.0;
	bool check = checkIndex(mixtureElement, 1, numMixtures);
	codon[0] = (char) std::toupper(codon[0]);
	codon[1] = (char) std::toupper(codon[1]);
	codon[2] = (char) std::toupper(codon[2]);
	if (check)
	{
		rv = getAlphaPosteriorMean(mixtureElement - 1, samples, codon);
	}
	return rv;
}

double PANSEParameter::getLambdaPrimePosteriorMeanForCodon(unsigned mixtureElement, unsigned samples, std::string codon)
{
	double rv = -1.0;
	codon[0] = (char) std::toupper(codon[0]);
	codon[1] = (char) std::toupper(codon[1]);
	codon[2] = (char) std::toupper(codon[2]);
	bool check = checkIndex(mixtureElement, 1, numMixtures);
	if (check)
	{
		rv = getLambdaPrimePosteriorMean(mixtureElement - 1, samples, codon);
	}
	return rv;
}


double PANSEParameter::getAlphaVarianceForCodon(unsigned mixtureElement, unsigned samples, std::string codon, bool unbiased)
{
	double rv = -1.0;
	codon[0] = (char) std::toupper(codon[0]);
	codon[1] = (char) std::toupper(codon[1]);
	codon[2] = (char) std::toupper(codon[2]);
	bool check = checkIndex(mixtureElement, 1, numMixtures);
	if (check)
	{
		rv = getAlphaVariance(mixtureElement - 1, samples, codon, unbiased);
	}
	return rv;
}


double PANSEParameter::getLambdaPrimeVarianceForCodon(unsigned mixtureElement, unsigned samples, std::string codon, bool unbiased)
{
	double rv = -1.0;
	codon[0] = (char) std::toupper(codon[0]);
	codon[1] = (char) std::toupper(codon[1]);
	codon[2] = (char) std::toupper(codon[2]);
	bool check = checkIndex(mixtureElement, 1, numMixtures);
	if (check)
	{
		rv = getLambdaPrimeVariance(mixtureElement - 1, samples, codon, unbiased);
	}
	return rv;
}


#ifndef STANDALONE
PANSEParameter::PANSEParameter(std::vector<double> sphi, std::vector<unsigned> geneAssignment, std::vector<unsigned> _matrix, bool splitSer) : Parameter(64)
{
  unsigned _numMixtures = _matrix.size() / 2;
  std::vector<std::vector<unsigned>> thetaKMatrix;
  thetaKMatrix.resize(_numMixtures);

  unsigned index = 0;
  for (unsigned i = 0; i < _numMixtures; i++)
  {
    for (unsigned j = 0; j < 2; j++, index++)
    {
      thetaKMatrix[i].push_back(_matrix[index]);
    }
  }
  initParameterSet(sphi, _matrix.size() / 2, geneAssignment, thetaKMatrix, splitSer);
  initPANSEParameterSet();

}

PANSEParameter::PANSEParameter(std::vector<double> sphi, unsigned _numMixtures, std::vector<unsigned> geneAssignment, bool splitSer, std::string _mutationSelectionState) :
Parameter(64)
{
  std::vector<std::vector<unsigned>> thetaKMatrix;
  initParameterSet(sphi, _numMixtures, geneAssignment, thetaKMatrix, splitSer, _mutationSelectionState);
  initPANSEParameterSet();
}
#endif
