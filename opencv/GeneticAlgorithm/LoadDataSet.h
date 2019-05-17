#pragma once
#ifndef LOAD_DATA_SET_H
#define LOAD_DATA_SET_H
#include "Matrix.h"
#include <string>
#include <fstream>
#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

struct TestElement
{
	vector<Mat> inputs;
	Matrix<int> output;
};

bool loadFolder(const string folderName, TestElement &testElement)
{
	int index = 0;

	Mat img = imread(folderName + '\\' + to_string(index) + ".jpg");

	if (!img.data)
		return false;

	do
	{
		testElement.inputs.push_back(img);

		index++;
		img = imread(folderName + '\\' + to_string(index) + ".jpg");
	} while (img.data);

	const string path = folderName + '\\' + "output.txt";
	testElement.output.fromFile(path.c_str());
	return true;
}


vector<TestElement> loadDataSet()
{
	int folderIndex = 0;
	vector<TestElement> dataSet;
	do
	{
		dataSet.push_back(TestElement());
	} while (loadFolder(to_string(folderIndex++), dataSet.back()));
	dataSet.pop_back();
	return dataSet;
}
#endif