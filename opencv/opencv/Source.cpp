#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/imgproc/imgproc.hpp>
#include <queue>
#include <map>
#include <string> 
#include <iostream>
#include "Matrix.h"
#include "CrosswordDetection.h"

#define DATA_INPUT	"0\\1.jpg"
#define DATA_OUTPUT	"0\\output.txt"

using namespace cv;
using namespace std;


int main()
{
	const string inputFileName = DATA_INPUT;
	const string outputFileName = DATA_OUTPUT;
	Mat img = imread(inputFileName), resized, canny, l2r, u2d, crossword;

	if (!img.data)
	{
		cout << "Nie odnalezionu pliku " << inputFileName << endl;
		system("PAUSE");
		return -1;
	}

	Matrix<int> testMatrix;
	if (!testMatrix.fromFile(outputFileName.c_str()))
	{
		cout << "Nie odnaleziono pliku " << outputFileName << endl;
		system("PAUSE");
		return -2;
	}

	resize(img, resized, { 900, 900 });
	
	namedWindow("image", CV_WINDOW_AUTOSIZE);
	imshow("image", resized);
	waitKey(0);

	erode(resized, resized, Mat(), Point(-1, -1), 3);

	imshow("image", resized);
	waitKey(0);

	Canny(resized, canny, 50, 150);

	for (int i = 0; i < 3; i++)
	{
		dilate(canny, canny, Mat(), Point(-1, -1), 5);
		erode(canny, canny, Mat(), Point(-1, -1), 5);
	}
	dilate(canny, canny, Mat(), Point(-1, -1), 2);

	imshow("image", canny);
	waitKey(0);

	detectCrosswordLeftToRigth(canny, l2r);
	detectCrosswordUpToDown(canny, u2d);

	imshow("image", l2r);
	waitKey(0);

	imshow("image", u2d);
	waitKey(0);

	bitwise_and(l2r, u2d, crossword);

	imshow("image", crossword);
	waitKey(0);

	eraseGray(crossword, crossword);

	imshow("image", crossword);
	waitKey(0);

	erode(crossword, crossword, Mat(), Point(-1, -1), 1);

	dilate(crossword, crossword, Mat(), Point(-1, -1), 5);

	frameGray(crossword, crossword);

	imshow("image", crossword);
	waitKey(0);

	dilate(crossword, crossword, Mat(), Point(-1, -1), 7);

	imshow("image", crossword);
	waitKey(0);

	queue<Point> points = indexGray(crossword);
	int boxesCount = points.size();
	Matrix<int> matrix;
	if (boxesCount != 0)
	{

		CrosswordBox *crosswordBoxes = new CrosswordBox[boxesCount];

		for (int i = 0; i < boxesCount; i++)
		{
			crosswordBoxes[i] = getCrosswordBox(crossword, points.front());
			points.pop();
		}

		setXYinCrosswordBoxes(crosswordBoxes, boxesCount);
		Point size = getMatrixSize(crosswordBoxes, boxesCount);

		matrix = Matrix<int>(size.x, size.y);

		for (int i = 0; i < boxesCount; i++)
		{
			if (crosswordBoxes[i].point.x == 0x80000000)
				continue;

			matrix.at(crosswordBoxes[i].point.x, crosswordBoxes[i].point.y) = 1;
		}
		delete[] crosswordBoxes;
	}
	else
	{
		matrix = Matrix<int>(1, 1);
	}

	cout << "Output z data setu:" << endl;

	for (int i = 0; i < testMatrix.getM(); i++)
	{
		for (int j = 0; j < testMatrix.getN(); j++)
		{
			cout << testMatrix.at(j, i);
		}
		cout << endl;
	}

	cout << endl << endl << "Output computera:" << endl;

	for (int i = 0; i < matrix.getM(); i++)
	{
		for (int j = 0; j < matrix.getN(); j++)
		{
			cout << matrix.at(j, i);
		}
		cout << endl;
	}

	cout << endl;

	cout << matrix.evaluateIdentity(testMatrix) << endl;

	waitKey(0);
	return 0;
}