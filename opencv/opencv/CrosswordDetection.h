#pragma once
#ifndef CROSSWORD_DETECTION_H
#define CROSSWORD_DETECTION_H
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <queue>

using namespace std;
using namespace cv;

struct CrosswordBox;

void detectCrosswordLeftToRigth(const Mat &src, Mat &dest);

void detectCrosswordUpToDown(const Mat &src, Mat &dest);

void extendWhiteFrame(const Mat &src, Mat &dest, int iteration);

void eraseGray(const Mat &src, Mat &dest);

queue<Point> indexGray(Mat &src);

CrosswordBox getCrosswordBox(Mat &src, Point point);

void setXYinCrosswordBoxes(CrosswordBox *crosswordBoxes, int size);

Point getMatrixSize(CrosswordBox *crosswordBoxes, int n);

void frameGray(const Mat &src, Mat &dest);


struct CrosswordBox
{
	int id;
	int neighborhoodIds[4];
	Point point;
};
#endif