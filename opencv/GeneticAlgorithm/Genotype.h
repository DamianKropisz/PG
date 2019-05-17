#pragma once
#ifndef GENOTYPE_H
#define GENOTYPE_H
#include "Chromosome.h"
#include <vector>
#include <list>
#include "LoadDataSet.h"
#include <math.h>


class Genotype
{
	friend ostream &operator<<(ostream &out, const Genotype &genotype);
public:
	Genotype()
		: value(0)
	{ }

	double getValue() const { return value; }

	void generate()
	{
		for (int i = 0; i < sizeof(specialChromosomes) / sizeof(specialChromosomes[0]); i++)
		{
			specialChromosomes[i] = getSpecialChromosomeWithRandomParam(i);
			for (int j = 0; j < sizeof(chromosomes[0]) / sizeof(chromosomes[0][0]); j++)
			{
				chromosomes[i][j] = getRandomChromosome();
			}
		}

		for (int j = 0; j < sizeof(chromosomes[0]) / sizeof(chromosomes[0][0]); j++)
		{
			chromosomes[sizeof(specialChromosomes) / sizeof(specialChromosomes[0])][j] = getRandomChromosome();
		}
	}

	static pair<Genotype, Genotype> generateWithCrossover(Genotype &g1, Genotype &g2)
	{
		pair<Genotype, Genotype> pair;
		int cross = rand() % (sizeof(specialChromosomes) / sizeof(specialChromosomes[0])
			+ sizeof(chromosomes) / sizeof(chromosomes[0][0]) - 2) + 1;
		int count = 0;
		Genotype *pParent[2] = { &g1, &g2 };

		for (int i = 0; i < sizeof(chromosomes) / sizeof(chromosomes[0]); i++)
		{

			for (int j = 0; j < sizeof(chromosomes[0]) / sizeof(chromosomes[0][0]); j++)
			{
				pair.first.chromosomes[i][j] = pParent[0]->chromosomes[i][j];
				pair.second.chromosomes[i][j] = pParent[1]->chromosomes[i][j];
				count++;
				if (count == cross)
					swap(pParent[0], pParent[1]);
			}

			if (i < sizeof(specialChromosomes) / sizeof(specialChromosomes[0]))
			{
				pair.first.specialChromosomes[i] = pParent[0]->specialChromosomes[i];
				pair.second.specialChromosomes[i] = pParent[1]->specialChromosomes[i];
				count++;
				if (count == cross)
					swap(pParent[0], pParent[1]);
			}
		}

		if (rand() % 10 == 0)
			pair.first.mutate();

		if (rand() % 10 == 0)
			pair.second.mutate();

		return pair;
	}


	static Genotype generateWithMutation(const Genotype &parent)
	{
		Genotype genotype;
		for (int i = 0; i < sizeof(specialChromosomes) / sizeof(specialChromosomes[0]); i++)
		{
			genotype.specialChromosomes[i] = rand() % 2
				? parent.specialChromosomes[i]
				: getSpecialChromosomeWithRandomParam(i);
		}

		for (int i = 0; i < sizeof(chromosomes) / sizeof(chromosomes[0]); i++)
		{
			for (int j = 0; j < sizeof(chromosomes[0]) / sizeof(chromosomes[0][0]); j++)
			{
				genotype.chromosomes[i][j] = rand() % 2
					? parent.chromosomes[i][j]
					: getRandomChromosome();
			}
		}

		return genotype;
	}


	void mutate()
	{
		int r1 = rand() % (sizeof(chromosomes) / sizeof(chromosomes[0][0]));
		int r2 = rand() % (sizeof(chromosomes) / sizeof(chromosomes[0][0]));

		Chromosome *chr = chromosomes[0];
		swap(chr[r1], chr[r2]);
	}


	double evaluate(const vector<TestElement> &dataSets)
	{
		double rate = 0;
		int i = 0;
		for each (const TestElement &ts in dataSets)
		{
			for each(const Mat &mat in ts.inputs)
			{
				const Matrix<int> genotypeOutput = getGenotypeOutput(mat);
				rate += genotypeOutput.evaluateIdentity(ts.output);
				i++;
			}
		}
		return value = rate / i;
	}

private:
	Chromosome specialChromosomes[sizeof(pPrototypeSpecialChromosome) / sizeof(pPrototypeSpecialChromosome[0])];
	Chromosome chromosomes[sizeof(pPrototypeSpecialChromosome) / sizeof(pPrototypeSpecialChromosome[0]) + 1][10];
	double value;

	Matrix<int> getGenotypeOutput(const Mat &input) const
	{
		list<Mat> list;
		list.push_back(Mat());
		const int size = sizeof(chromosomes[0]) / sizeof(chromosomes[0][0]);
		chromosomes[0][0].call(input, list.back());

		for (int i = 1; i < size; i++)
		{
			list.push_back(Mat());
			chromosomes[0][i].call(list.front(), list.back());
			list.pop_front();
		}

		for (int i = 0; i < sizeof(specialChromosomes) / sizeof(specialChromosomes[0]); i++)
		{
			list.push_back(Mat());
			specialChromosomes[i].call(list.front(), list.back());
			list.pop_front();

			for (int j = 0; j < sizeof(chromosomes[0]) / sizeof(chromosomes[0][0]); j++)
			{
				list.push_back(Mat());
				chromosomes[i + 1][j].call(list.front(), list.back());
				list.pop_front();
			}
		}

		queue<Point> points = indexGray(list.front());
		int boxesCount = points.size();

		if (boxesCount == 0)
			return Matrix<int>(1, 1);

		CrosswordBox *crosswordBoxes = new CrosswordBox[boxesCount];


		for (int i = 0; i < boxesCount; i++)
		{
			crosswordBoxes[i] = getCrosswordBox(list.front(), points.front());
			points.pop();
		}

		setXYinCrosswordBoxes(crosswordBoxes, boxesCount);
		Point point = getMatrixSize(crosswordBoxes, boxesCount);
		
		Matrix<int> matrix(point.x, point.y);

		for (int i = 0; i < boxesCount; i++)
		{
			if (crosswordBoxes[i].point.x == 0x80000000)
				continue;

			matrix.at(crosswordBoxes[i].point.x, crosswordBoxes[i].point.y) = 1;
		}

		return matrix;
	}
};

ostream &operator<<(ostream &out, const Genotype &genotype)
{
	out << genotype.value;
	for (int i = 0; i < sizeof(Genotype::specialChromosomes) / sizeof(Genotype::specialChromosomes[0]); i++)
	{
		out << ' ' << genotype.specialChromosomes[i];
	}

	for (int i = 0; i < sizeof(Genotype::chromosomes) / sizeof(Genotype::chromosomes[0]); i++)
	{
		for (int j = 0; j < sizeof(Genotype::chromosomes[0]) / sizeof(Genotype::chromosomes[0][0]); j++)
		{
			out << ' ' << genotype.chromosomes[i][j];
		}
	}
	return out;
}
#endif