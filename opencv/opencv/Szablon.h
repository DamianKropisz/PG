#pragma once
#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/imgproc/imgproc.hpp>
#include "Matrix.h"

using namespace std;
using namespace cv;

//przyk³adowa struktura TestElement, wybierz jedn¹
struct TestElement
{
	vector<Mat> inputs;
	Matrix<bool> output; //output.fromFile("output.txt");
};

struct TestElement2
{
	Mat input;
	Matrix<bool> output;
};


void fun(const string &folderName)
{
	int index = 1;

	Mat img = imread(folderName + '\\' + to_string(index) + ".jpg");

	while (img.data)
	{
		// rób coœtam

		index++;
		img = imread(folderName + '\\' + to_string(index) + ".jpg");
	}

	Matrix<bool> matrix;
	const string path = folderName + '\\' + "output.txt";
	matrix.fromFile(path.c_str());
}