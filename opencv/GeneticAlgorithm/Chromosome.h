#pragma once
#ifndef CHROMOSOME_H
#define CHROMOSOME_H
#include "CrosswordDetection.h"
#include <functional>
#include <iostream>
#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;


class Chromosome
{
	friend ostream &operator<<(ostream &out, const Chromosome &chromosome);
public:
	Chromosome()
		: pParam(nullptr)
	{ }

	Chromosome(int ID,
		function<void(const Mat&, Mat&, const void*)> pFunc,
		function<const void*()> pParamGenerator,
		function<const void*(const void*)> pCopyParameters,
		function<void(const void*)> pDealloc,
		function<ostream&(ostream&, const Chromosome&)> pOstreamHandler)
		: ID(ID),
		pFunc(pFunc),
		pGetRandomParameters(pParamGenerator),
		pCopyParameters(pCopyParameters),
		pDealloc(pDealloc),
		pOstreamHandler(pOstreamHandler),
		pParam(nullptr)
	{ }

	Chromosome(const Chromosome& copy)
		: ID(copy.ID),
		pFunc(copy.pFunc),
		pGetRandomParameters(copy.pGetRandomParameters),
		pCopyParameters(copy.pCopyParameters),
		pDealloc(copy.pDealloc),
		pOstreamHandler(copy.pOstreamHandler)
	{
		if (copy.pParam != nullptr)
			pParam = copy.pCopyParameters(copy.pParam);
		else
			pParam = nullptr;
	}

	~Chromosome()
	{
		if (pParam != nullptr)
			pDealloc(pParam);
	}

	int getID() const { return ID; }

	const void *getPParam() const { return pParam; }

	void call(const Mat &src, Mat &dest) const
	{
		pFunc(src, dest, pParam);
	}

	void setRandomParameter()
	{
		pParam = pGetRandomParameters();
	}

	Chromosome &operator=(const Chromosome &copy)
	{
		if (pParam != nullptr)
			pDealloc(pParam);

		ID = copy.ID;
		pFunc = copy.pFunc;
		pGetRandomParameters = copy.pGetRandomParameters;
		pCopyParameters = copy.pCopyParameters;
		pDealloc = copy.pDealloc;
		pParam = pCopyParameters(copy.pParam);
		pOstreamHandler = copy.pOstreamHandler;
		return *this;
	}

private:
	int ID;
	function<void(const Mat&, Mat&, const void*)> pFunc;
	function<const void*()> pGetRandomParameters;
	function<const void*(const void*)> pCopyParameters;
	function<void(const void*)> pDealloc;
	function<ostream&(ostream&, const Chromosome&)> pOstreamHandler;
	const void *pParam;
};


ostream &operator<<(ostream &out, const Chromosome &chromosome)
{
	return chromosome.pOstreamHandler(out, chromosome);
}

const int params1[] = { 1, 2, 3, 4 };

const Chromosome prototypeChromosomes[] =
{
	{
		0,
		[](const Mat &src, Mat &dest, const void*)->void { src.copyTo(dest); },
		[]()->const void* { return nullptr; },
		[](const void*)->const void* { return nullptr; },
		[](const void*)->void { },
		[](ostream &out, const Chromosome &chromosome)->ostream& { return out << chromosome.getID(); }
	},
	{
		1,
		[](const Mat &src, Mat &dest, const void *pParam)->void
		{
			erode(src, dest, Mat(), Point(-1, -1), *(const int*)pParam);
		},
		[]()->const void* { return params1 + (rand() % 4); },
		[](const void *pParam)->const void* { return pParam; },
		[](const void*)->void { },
		[](ostream &out, const Chromosome &chromosome)->ostream&
		{
			return out << chromosome.getID() << ' ' << *(const int*)chromosome.getPParam();
		}
	},
	{
		2,
		[](const Mat &src, Mat &dest, const void *pParam)->void
		{
			dilate(src, dest, Mat(), Point(-1, -1), *(const int*)pParam);
		},
		[]()->const void* { return params1 + (rand() % 4); },
		[](const void *pParam)->const void* { return pParam; },
		[](const void*)->void { },
		[](ostream &out, const Chromosome &chromosome)->ostream&
		{
			return out << chromosome.getID() << ' ' << *(const int*)chromosome.getPParam();
		}
	},
	{
		3,
		[](const Mat &src, Mat &dest, const void *pParam)->void
		{
			extendWhiteFrame(src, dest, *(const int*)pParam);
		},
		[]()->const void* { return params1 + (rand() % 4); },
		[](const void *pParam)->const void* { return pParam; },
		[](const void*)->void { },
		[](ostream &out, const Chromosome &chromosome)->ostream&
		{
			return out << chromosome.getID() << ' ' << *(const int*)chromosome.getPParam();
		}
	}
};


const Chromosome prototypeResizeChromosome =
{
	4,
	[](const Mat &src, Mat &dest, const void *pParam)->void
	{
		int* params = (int*)pParam;
		resize(src, dest, { params[0], params[1] });
	},
	[]()->const void*
	{
		int *p = new int[2];
		p[0] = rand() % 900 + 300;
		p[1] = rand() % 900 + 300;
		return p;
	},
	[](const void *pParam)->const void*
	{
		int *p = new int[2];
		const int *r = (const int*)pParam;
		p[0] = r[0];
		p[1] = r[1];
		return p;
	},
	[](const void *pParam)->void { delete[](const int*)pParam; },
	[](ostream &out, const Chromosome &chromosome)->ostream&
	{
		const int *r = (const int*)chromosome.getPParam();
		return out << chromosome.getID() << ' ' << r[0] << ' ' << r[1];
	}
};


const Chromosome prototypeCannyChromosome = 
{
	5,
	[](const Mat &src, Mat &dest, const void *pParam)->void
	{
		double* params = (double*)pParam;
		Canny(src, dest, params[0], params[1]);
	},
	[]()->const void*
	{
		double *p = new double[2];
		p[0] = rand() / (double)(RAND_MAX) * 250;
		p[1] = rand() / (double)(RAND_MAX) * (250 - p[0]) + p[0];
		return p;
	},
	[](const void *pParam)->const void*
	{
		double *p = new double[2];
		const double *r = (const double*)pParam;
		p[0] = r[0];
		p[1] = r[1];
		return p;
	},
	[](const void *pParam)->void { delete[](const double*)pParam; },
	[](ostream &out, const Chromosome &chromosome)->ostream&
	{
		const double *r = (const double*)chromosome.getPParam();
		return out << chromosome.getID() << ' ' << r[0] << ' ' << r[1];
	}
};


const Chromosome prototypeCrosswordDetectionChromosome = 
{
	6,
	[](const Mat &src, Mat &dest, const void *pParam)->void
	{
		Mat l2r, u2d;
		detectCrosswordLeftToRigth(src, l2r);
		detectCrosswordUpToDown(src, u2d);
		bitwise_and(l2r, u2d, dest);
	},
	[]()->const void* { return nullptr; },
	[](const void*)->const void* { return nullptr; },
	[](const void*)->void { },
	[](ostream &out, const Chromosome &chromosome)->ostream& { return out << chromosome.getID(); }
};


const Chromosome prototypeEraseGrayChromosome = 
{
	7,
	[](const Mat &src, Mat &dest, const void *pParam)->void
	{
		eraseGray(src, dest);
	},
	[]()->const void* { return nullptr; },
	[](const void*)->const void* { return nullptr; },
	[](const void*)->void { },
	[](ostream &out, const Chromosome &chromosome)->ostream& { return out << chromosome.getID(); }
};


const Chromosome prototypeFrameGrayChromosome =
{
	8,
	[](const Mat &src, Mat &dest, const void *pParam)->void
	{
		frameGray(src, dest);
	},
	[]()->const void* { return nullptr; },
	[](const void*)->const void* { return nullptr; },
	[](const void*)->void { },
	[](ostream &out, const Chromosome &chromosome)->ostream& { return out << chromosome.getID(); }
};


const Chromosome *pPrototypeSpecialChromosome[] =
{
	&prototypeResizeChromosome,
	&prototypeCannyChromosome,
	&prototypeCrosswordDetectionChromosome,
	&prototypeEraseGrayChromosome,
	&prototypeFrameGrayChromosome
};


Chromosome getSpecialChromosomeWithRandomParam(int i)
{
	Chromosome chrom = *pPrototypeSpecialChromosome[i];
	chrom.setRandomParameter();
	return chrom;
}


Chromosome getChromosomeWithRandomParam(int i)
{
	Chromosome chrom = prototypeChromosomes[i];
	chrom.setRandomParameter();
	return chrom;
}


Chromosome getRandomChromosome()
{
	return getChromosomeWithRandomParam(rand() % (sizeof(prototypeChromosomes) / sizeof(prototypeChromosomes[0])));
}
#endif