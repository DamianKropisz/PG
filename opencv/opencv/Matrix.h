#pragma once
#ifndef MATRIX_H
#define MATRIX_H
#include <stdlib.h>
#include <fstream>
template<typename TType>
class Matrix
{
public:
	Matrix()
		: tab(nullptr), n(0), m(0)
	{ }

	Matrix(int n, int m)
		: n(n), m(m)
	{
		tab = (TType*)calloc(n * m, sizeof(TType));
	}

	Matrix(const Matrix &copy)
		: n(copy.n), m(copy.m)
	{
		tab = (TType*)malloc(n * m * sizeof(TType));
		memcpy(tab, copy.tab, n * m * sizeof(TType));
	}

	~Matrix()
	{
		free(tab);
	}

	Matrix &operator=(const Matrix &copy)
	{
		n = copy.n;
		m = copy.m;
		
		if (copy.tab == nullptr)
		{
			tab = nullptr;
			return *this;
		}

		tab = (TType*)malloc(n * m * sizeof(TType));
		memcpy(tab, copy.tab, n * m * sizeof(TType));

		return *this;
	}

	TType& at(int x, int y)
	{
		if (x >= n || y >= m)
			throw bad_alloc();

		return tab[y * n + x];
	}

	TType at(int x, int y) const
	{
		if (x >= n || y >= m)
			throw bad_alloc();

		return tab[y * n + x];
	}

	int getN() const { return n; }
	int getM() const { return m; }

	bool toFile(const char *fileName)
	{
		fstream file;
		file.open(fileName, ios::out);

		if (!file.good())
			return false;

		file << n << ' ' << m;

		for (int i = 0; i < m; i++)
		{
			file << endl << tab[i * n];
			for (int j = 1; j < n; j++)
			{
				file << ' ' << tab[i * n + j];
			}
		}

		file.close();
		return true;
	}


	Matrix getCompareMatrix(const Matrix &rigth)
	{
		Matrix matrix(n, m);
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < m; j++)
			{
				matrix.at(i, j) = abs(at(i, j) - rigth.at(i, j));
			}
		}

		return matrix;
	}


	bool fromFile(const char *fileName)
	{
		free(tab);
		fstream file;
		file.open(fileName, ios::in);

		if (!file.good())
			return false;

		file >> n >> m;

		tab = (TType*)malloc(sizeof(TType) * n * m);

		for (int i = 0; i < m; i++)
		{
			for (int j = 0; j < n; j++)
			{
				file >> tab[i * n + j];
			}
		}

		file.close();
		return true;
	}

	int count(TType item) const
	{
		int count = 0;
		for (int i = 0; i < m; i++)
		{
			for (int j = 0; j < n; j++)
			{
				if (tab[i * n + j] == item)
					count++;
			}
		}
		return count;
	}

	void rotateLeft()
	{
		Matrix tmpMatrix(m, n);

		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < m; j++)
			{
				tmpMatrix.at(j, i) = at(n - i - 1, j);
			}
		}

		*this = tmpMatrix;
	}

	void rotate180()
	{
		for (int i = 1; i < n; i++)
		{
			for (int j = 0; j < (m + 1) / 2; j++)
			{
				swap(at(i, j), at(n - i - 1, m - j - 1));
			}
		}
	}

	double evaluateIdentity(const Matrix &matrix) const
	{
		if (matrix.n > n && matrix.m > m)
			return matrix.evaluateIdentity(*this);

		TType countBox = 0;
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < m; j++)
			{
				countBox += at(i, j);
			}
		}

		double max = 0;
		for (int i = 0; i < n - matrix.n + 1; i++)
		{
			for (int j = 0; j < m - matrix.m + 1; j++)
			{
				Matrix mat(n, m);

				for (int k = 0; k < matrix.n; k++)
				{
					for (int l = 0; l < matrix.m; l++)
					{
						mat.at(i + k, j + l) = matrix.at(k, l);
					}
				}

				mat.absDiff(*this);
				TType sum = mat.sum();
				double tmp = sum / 1.0 / countBox;
				tmp = 0 - (tmp - 0.5) + 0.5;
				if (tmp > max)
					max = tmp;
			}
		}
		return max;
	}


	Matrix &absDiff(const Matrix &matrix)
	{
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < m; j++)
			{
				at(i, j) = abs(matrix.at(i, j) - at(i, j));
			}
		}

		return *this;
	}


	TType sum() const
	{
		TType sum = 0;
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < m; j++)
			{
				sum += at(i, j);
			}
		}

		return sum;
	}

	bool operator==(const Matrix &matrix)
	{
		if (n != matrix.n || m != matrix.m)
			goto next2;

		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < m; j++)
			{
				if (at(i, j) != matrix.at(i, j))
					goto next;
			}
		}

		return true;
	next:
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < m; j++)
			{
				if (at(i, j) != matrix.at(n - i - 1, m - j - 1))
					goto next2;
			}
		}

		return true;

	next2:
		if (n != matrix.m || m != matrix.n)
			return false;

		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < m; j++)
			{
				if (at(i, j) != matrix.at(j, i))
					goto next4;
			}
		}

		return true;
	next4:
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < m; j++)
			{
				if (at(i, j) != matrix.at(n - j - 1, n - i - 1))
					return false;
			}
		}
		return true;
	}

	bool operator!=(const Matrix &matrix) { return !this->operator==(matrix); }

private:
	TType *tab;
	int n, m;
};
#endif