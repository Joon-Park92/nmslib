#include <limits>

#include "trigen/cTriGen.h"
#include "trigen/cUniformRandomGenerator.h"

//#include <float.h>
#include "logging.h"

const double DBL_MAX = numeric_limits<double>::max();

cTriGen::cTriGen(cDistance* distance, cArray<cDataObject> &source, unsigned int sampleSize, cArray<cSPModifier*> &modifBases)
{
	unsigned int len = sampleSize * sampleSize;

	mDistanceMatrix = new double [len];
	for(unsigned int i=0; i<len; i++)
		mDistanceMatrix[i]=-1;

	mItems = new cDataObject* [sampleSize];
	mCount = sampleSize;

	SampleItems(source);

	mModifierBases = modifBases;

	mDistance = distance;
}

cTriGen::~cTriGen(void)
{	
	delete [] mDistanceMatrix;	
	delete [] mItems;
}

int cTriGen::compareOrderIndexPairs(const void* item1, const void* item2)
{
	return ((order_index_pair*)item1)->order < ((order_index_pair*)item2)->order;
}

void cTriGen::SampleItems(cArray<cDataObject> &source)
{
	cUniformRandomGenerator gen(17);
	unsigned int datasetSize = source.Count();

	order_index_pair* reordering_array = new order_index_pair [source.Count()];

	for(unsigned int i = 0; i < source.Count(); i++)
	{
		reordering_array[i].order = (int)(gen.GetNext() * source.Count());
		reordering_array[i].index = i;
	}

	qsort((void*)reordering_array, source.Count(), sizeof(order_index_pair), compareOrderIndexPairs);

	for(unsigned int i = 0; i < mCount; i++)
	{
		mItems[i] = &source[reordering_array[i].index];
	}

	delete [] reordering_array;
}

double cTriGen::ComputeTriangleError(unsigned int tripletSampleCount, double errorToleranceSkip, eSamplingTriplets samplingTriplets)
{		
	cUniformRandomGenerator gen(13);
	unsigned int a,b,c;
	cOrderedTriplet triplet;

	unsigned int nonTriangularCount;
	nonTriangularCount = 0;

	unsigned int errorThreshold = (int)((double)tripletSampleCount*errorToleranceSkip);

	for(unsigned int i = 0; i < tripletSampleCount ; )
	{	
		a = (unsigned int)((mCount - 1) * gen.GetNext());

		//if (samplingTriplets == eRandom)
		{			
			b = (unsigned int)((mCount - 1) * gen.GetNext()); 
			c = (unsigned int)((mCount - 1) * gen.GetNext());
		} 

		triplet.SetTriplet(GetModifiedDistance(a,b), GetModifiedDistance(b,c), GetModifiedDistance(a,c));
		if (triplet.isRegular())
		{
			i++;
			if (!triplet.isTriangular())
			{
				nonTriangularCount++;
				if (nonTriangularCount > errorThreshold)
					return (double)nonTriangularCount/tripletSampleCount;
			}
		}
	}
	
	return (double)nonTriangularCount/tripletSampleCount;
}

void cTriGen::ComputeDistribution(unsigned int distanceSampleCount, double& mean, double& variance, double& idim)
{
	mean = variance = 0;
	double dist;

	cUniformRandomGenerator gen1(13), gen2(13);

	for(unsigned int i=0; i<distanceSampleCount; i++)
	{		
		dist = GetModifiedDistance((unsigned int)((mCount-1) * gen1.GetNext()),  (unsigned int)((mCount-1) * gen1.GetNext()));
		mean += dist;
	}		
	mean /= distanceSampleCount;

	for(unsigned int i=0; i<distanceSampleCount; i++)
	{		
		dist = GetModifiedDistance((unsigned int)((mCount-1) * gen2.GetNext()),  (unsigned int)((mCount-1) * gen2.GetNext()));
		variance += __SQR(dist - mean);
	}		
	variance /= distanceSampleCount;

	idim = __SQR(mean)/(2*variance);	
}

cSPModifier* cTriGen::Run(/* in/out */double& errorTolerance, /* out */unsigned int& funcOrder, /* out */ double& resultIDim, /* in */ unsigned int tripletSampleCount,  /* in */ bool echoOn, /* in */ eSamplingTriplets samplingTriplets)
{
	unsigned iterLimit = 24;
	double w_LB, w_UB, w_best, error, err_best = DBL_MAX, inputTolerance = errorTolerance;
	cSPModifier* result = NULL;

	double mean, variance, idim;
	resultIDim = DBL_MAX;
	//char infoBuffer[256];

	if (echoOn)
		LOG(LIB_INFO) << "TriGen started, " <<  mModifierBases.Count() << " bases are going to be tried, " << tripletSampleCount << " sampled triplets"; 

	for(unsigned int i=0; i<mModifierBases.Count(); i++)
	{
		w_LB = 0; w_UB = DBL_MAX; 		
		w_best = -1;

		mCurrentModifier = mModifierBases[i];					
		mCurrentModifier->SetConcavityWeight(1);
		if (echoOn)
		  LOG(LIB_INFO) << " Trying modifier: " << mCurrentModifier->GetInfo();

		for(unsigned int iter = 0; iter < iterLimit; iter++)
		{
			ClearModifiedDistances();			

			error = ComputeTriangleError(tripletSampleCount, inputTolerance, samplingTriplets);
			if (error <= inputTolerance)
			{
				w_UB = w_best = mCurrentModifier->GetConcavityWeight();
				err_best = error;
				mCurrentModifier->SetConcavityWeight((w_LB + w_UB)/2);				
			}
			else
			{
				w_LB = mCurrentModifier->GetConcavityWeight();
				if (w_UB != DBL_MAX)
					mCurrentModifier->SetConcavityWeight((w_LB + w_UB)/2);				
				else
					mCurrentModifier->SetConcavityWeight(2.0*mCurrentModifier->GetConcavityWeight());
			}
		}

		if (w_best > -1)
		{
			ClearModifiedDistances();
			mCurrentModifier->SetConcavityWeight(w_best);			
			ComputeDistribution(tripletSampleCount, mean, variance, idim);	
			
			if (idim < resultIDim)
			{
				funcOrder = i; result = mCurrentModifier; 
				resultIDim = idim;
				errorTolerance = err_best;
				if (echoOn)				
					LOG(LIB_INFO) << "found the best so far - idim: " << idim << " cw:  " << mCurrentModifier->GetConcavityWeight() << ", err: " <<  err_best;
			} else
			{
				if (echoOn)
				  LOG(LIB_INFO) << "found but not the best (idim: " << idim << ")";
			}

		} else
		{
			if (echoOn)
			  LOG(LIB_INFO) << "not found";
		}

	}
	return result;
}
