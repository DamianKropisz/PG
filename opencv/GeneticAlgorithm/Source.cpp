#include <Windows.h>
#include "Chromosome.h"
#include "LoadDataSet.h"
#include "Genotype.h"
#include <ctime>
#include <thread>
#include <mutex>
#include <string>
#include <condition_variable>

#define GENOTYPE_COUNT 300
#define SELECTED_GENOTYPES 40

Genotype genotypes[GENOTYPE_COUNT];
vector<TestElement> dataSet;
thread threads[3];
int counter = 0, threadCounter = 0;
bool isWaiting = false;
mutex cout_mutex, threadMutex;
condition_variable condi;

void sortGenotypes(int a, int b);
void mergeGenotypes(int a, int b, int c);
void generateGenotypes(int a, int b);
void generateGenotypesFromParents(int a, int b);


int main()
{
	time_t rawtime;
	tm timeInfo;
	char buff[80];
	time(&rawtime);
	localtime_s(&timeInfo, &rawtime);
	strftime(buff, sizeof(buff), "%d-%m-%Y %I.%M.%S", &timeInfo);
	string folderName(buff);
	fstream file;
	if (!CreateDirectory(folderName.c_str(), NULL) &&
		ERROR_ALREADY_EXISTS != GetLastError())
	{
		cout << "Cannot create directory " + folderName + "!!!" << endl;
		system("PAUSE");
		return -1;
	}
	cout << "Folder " + folderName + " created" << endl;
	file.open(folderName + "/GENERATION 0.txt", ios::out);
	if (!file.is_open())
	{
		cout << "Cannot open file GENERATION 0.txt to write!!!" << endl;
		system("PAUSE");
		return -1;
	}

	srand(time(NULL));
	rand();
	cout << "Loading data set" << endl;
	dataSet = loadDataSet();
	cout << "Data set loaded" << endl;
	cout << "GENERATION 0" << endl;
	unique_lock<mutex> ul(threadMutex);
	for (int i = 0; i < 3; i++)
	{
		threads[i] = thread([i]()
		{
			generateGenotypes((GENOTYPE_COUNT - 1) * i / 4 + 1, (GENOTYPE_COUNT - 1) * (i + 1) / 4);
			unique_lock<mutex> ul(threadMutex);
			if (--threadCounter == 0 && isWaiting)
				condi.notify_one();
			ul.unlock();
		});
	}
	threadCounter = 3;
	ul.unlock();
	generateGenotypes(0, 0);
	generateGenotypes((GENOTYPE_COUNT - 1) * 3 / 4 + 1, GENOTYPE_COUNT - 1);
	ul.lock();
	if (threadCounter != 0)
	{
		isWaiting = true;
		condi.wait(ul);
	}
	ul.unlock();

	for (int i = 0; i < 3; i++)
		threads[i].detach();

	sortGenotypes(0, GENOTYPE_COUNT - 1);

	file << genotypes[0];
	for (int i = 1; i < GENOTYPE_COUNT; i++)
	{
		file << endl << genotypes[i];
	}
	file.close();

	//algorithm
	int generation = 0;
	do
	{
		cout << endl << "GENERATION " << generation++ << " best value: " << genotypes[0].getValue() << endl;
		fstream file;
		file.open(folderName + "/GENERATION " + to_string(generation) + ".txt", ios::out);
		if (!file.is_open())
		{
			cout << "Cannot open file GENERATION " << generation << ".txt to write!!!" << endl;
			system("PAUSE");
			return -1;
		}

		cout << "GENERATION " << generation << endl;
		counter = 0;
		unique_lock<mutex> ul(threadMutex);
		isWaiting = false;
		for (int i = 0; i < 3; i++)
		{
			threads[i] = thread([i]()
			{
				generateGenotypesFromParents(SELECTED_GENOTYPES + (GENOTYPE_COUNT - SELECTED_GENOTYPES) * i / 4 + 1,
					SELECTED_GENOTYPES + (GENOTYPE_COUNT - SELECTED_GENOTYPES) * (i + 1) / 4);
				unique_lock<mutex> ul(threadMutex);
				if (--threadCounter == 0 && isWaiting)
					condi.notify_one();
				ul.unlock();
			});
		}
		threadCounter = 3;
		ul.unlock();
		generateGenotypesFromParents(SELECTED_GENOTYPES, SELECTED_GENOTYPES);
		generateGenotypesFromParents(SELECTED_GENOTYPES + (GENOTYPE_COUNT - SELECTED_GENOTYPES) * 3 / 4 + 1,
			GENOTYPE_COUNT - 1);
		ul.lock();
		if (threadCounter != 0)
		{
			isWaiting = true;
			condi.wait(ul);
		}
		ul.unlock();

		for (int i = 0; i < 3; i++)
			threads[i].detach();

		sortGenotypes(0, GENOTYPE_COUNT - 1);

		

		file << genotypes[0];
		for (int i = 1; i < GENOTYPE_COUNT; i++)
		{
			 file << endl << genotypes[i];
		}
		file.close();

	} while (true);

	return 0;
}


void generateGenotypes(int a, int b)
{
	for (int i = a; i <= b; i++)
	{
		genotypes[i].generate();
		genotypes[i].evaluate(dataSet);
		cout_mutex.lock();
		printf("\r%.2f%%", ++counter * 100.0 / GENOTYPE_COUNT);
		cout_mutex.unlock();
	}
}


void generateGenotypesFromParents(int a, int b)
{
	for (int i = a; i <= b; i+=3)
	{
		int r1 = rand() % (SELECTED_GENOTYPES), r2 = rand() % (SELECTED_GENOTYPES);
		pair<Genotype, Genotype> pair = Genotype::generateWithCrossover(genotypes[r1], genotypes[r2]);
		genotypes[i] = pair.first;
		genotypes[i].evaluate(dataSet);
		cout_mutex.lock();
		printf("\r%.2f%%", ++counter * 100.0 / (GENOTYPE_COUNT - SELECTED_GENOTYPES));
		cout_mutex.unlock();

		if (i + 1 <= b)
		{
			genotypes[i + 1] = pair.second;
			genotypes[i + 1].evaluate(dataSet);
			cout_mutex.lock();
			printf("\r%.2f%%", ++counter * 100.0 / (GENOTYPE_COUNT - SELECTED_GENOTYPES));
			cout_mutex.unlock();
		}

		if (i + 2 <= b)
		{
			r1 = rand() % SELECTED_GENOTYPES;
			genotypes[i + 2] = Genotype::generateWithMutation(genotypes[r1]);
			genotypes[i + 2].evaluate(dataSet);
			cout_mutex.lock();
			printf("\r%.2f%%", ++counter * 100.0 / (GENOTYPE_COUNT - SELECTED_GENOTYPES));
			cout_mutex.unlock();
		}
	}
}


Genotype tmpGenotypes[GENOTYPE_COUNT];
void mergeGenotypes(int a, int b, int c)
{
	int i = a, j = c + 1, k = a;

	while (i <= c && j <= b)
	{
		if (genotypes[i].getValue() > genotypes[j].getValue())
		{
			tmpGenotypes[k++] = genotypes[i++];
		}
		else
		{
			tmpGenotypes[k++] = genotypes[j++];
		}
	}

	while (i <= c)
		tmpGenotypes[k++] = genotypes[i++];

	while (j <= b)
		tmpGenotypes[k++] = genotypes[j++];

	for (int i = a; i <= b; i++)
	{
		genotypes[i] = tmpGenotypes[i];
	}
}


void subSortGenotypes(int a, int b)
{
	if (b - a <= 4)
	{
		for (int i = a; i < b; i++)
		{
			int j_max = i;
			for (int j = i + 1; j <= b; j++)
			{
				if (genotypes[j_max].getValue() < genotypes[j].getValue())
					j_max = j;
			}
			swap(genotypes[i], genotypes[j_max]);
		}
		return;
	}

	int c = (a + b) / 2;

	subSortGenotypes(a, c);
	subSortGenotypes(c + 1, b);

	mergeGenotypes(a, b, c);
}

unsigned char threadState[3];
condition_variable sort_cv[3];
void sortGenotypes(int a, int b)
{
	subSortGenotypes(a, b);
	/*unique_lock<mutex> ul(threadMutex);
	threadState[0] = threadState[1] = threadState[2] = 0;
	ul.unlock();
	threads[0] = thread([a, b] ()
	{
		subSortGenotypes(a, (b - a) / 4 + a);
		unique_lock<mutex> ul(threadMutex);
		if (threadState[0] == 0)
		{
			threadState[0] = 1;
		}
		else
		{
			sort_cv[0].notify_one();
		}
		ul.unlock();
	});
	threads[1] = thread([a, b]()
	{
		subSortGenotypes((b - a) / 4 + a + 1, (a + b) / 2);
		unique_lock<mutex> ul(threadMutex);
		if (threadState[0] == 0)
		{
			threadState[0] = 1;
			sort_cv[0].wait(ul);
		}
		ul.unlock();

		mergeGenotypes(a, (b - a) / 4 + a, (a + b) / 2);

		ul.lock();
		if (threadState[1] == 0)
		{
			threadState[1] = 1;
		}
		else
		{
			sort_cv[1].notify_one();
		}
		ul.unlock();
	});
	threads[2] = thread([a, b]()
	{
		subSortGenotypes((a + b) / 2 + 1, (b - a) * 3 / 4 + a);
		unique_lock<mutex> ul(threadMutex);
		if (threadState[2] == 0)
		{
			threadState[2] = 1;
		}
		else
		{
			sort_cv[2].notify_one();
		}
		ul.unlock();
	});
	
	subSortGenotypes((b - a) * 3 / 4 + a + 1, b);
	ul.lock();
	if (threadState[2] == 0)
	{
		threadState[2] = 1;
		sort_cv[2].wait(ul);
	}
	ul.unlock();
	mergeGenotypes((a + b) / 2 + 1, b, (b - a) * 3 / 4 + a);
	ul.lock();
	if (threadState[1] == 0)
	{
		threadState[1] = 1;
		sort_cv[1].wait(ul);
	}
	ul.unlock();
	mergeGenotypes(a, b, (a + b) / 2);
	for (int i = 0; i < 3; i++)
		threads[i].detach();*/
}