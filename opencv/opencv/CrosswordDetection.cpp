#include "CrosswordDetection.h"
#include <map>
#include <functional>

#define LEFT 0
#define RIGTH 1
#define UP 2
#define DOWN 3

#define GRAY 254

static const Point directions[4] = { Point(-1, 0), Point(1, 0), Point(0, -1), Point(0, 1) };

void detectCrosswordLeftToRigth(const Mat& src, Mat& dest)
{
	dest = Mat(src.rows, src.cols, CV_8UC1, Scalar(0));
	for (int i = 0; i < src.rows; i++)
	{
		int last;
		for (int j = 0; j < src.cols;)
		{
			last = j;
			for (; j < src.cols; j++)
			{
				if (src.at<uchar>(i, j) != 0)
				{
					if (last != 0)
						for (int k = last; k < j; k++)
						{
							dest.at<uchar>(i, k) = 255;
						}
					else
						for (int k = last; k < j; k++)
						{
							dest.at<uchar>(i, k) = GRAY;
						}
					break;
				}
			}

			if (j == src.cols)
				for (int k = last; k < j; k++)
				{
					dest.at<uchar>(i, k) = GRAY;
				}

			for (; j < src.cols && src.at<uchar>(i, j) == 255; j++)
			{
			}
		}
	}
}


void detectCrosswordUpToDown(const Mat& src, Mat& dest)
{
	dest = Mat(src.rows, src.cols, CV_8UC1, Scalar(0));
	for (int i = 0; i < src.cols; i++)
	{
		int last;
		for (int j = 0; j < src.rows;)
		{
			last = j;
			for (; j < src.rows; j++)
			{
				if (src.at<uchar>(j, i) != 0)
				{
					if (last != 0)
						for (int k = last; k < j; k++)
						{
							dest.at<uchar>(k, i) = 255;
						}
					else
						for (int k = last; k < j; k++)
						{
							dest.at<uchar>(k, i) = GRAY;
						}
					break;
				}
			}

			if (j == src.rows)
				for (int k = last; k < j; k++)
				{
					dest.at<uchar>(k, i) = GRAY;
				}

			for (; j < src.rows && src.at<uchar>(j, i) == 255; j++)
			{
			}
		}
	}
}


void extendWhiteFrame(const Mat &src, Mat &dest, int iteration)
{
	src.copyTo(dest);
	queue<Point> que;
	for (int i = 0; i < iteration; i++)
	{
		for (int j = 0; j < dest.rows; j++)
		{
			for (int k = 0; k < dest.cols; k++)
			{
				if (dest.at<uchar>(j, k) == 0)
					if ((j - 1 >= 0 && dest.at<uchar>(j - 1, k) == 255)
						|| (j + 1 < dest.rows && dest.at<uchar>(j + 1, k) == 255)
						|| (k - 1 >= 0 && dest.at<uchar>(j, k - 1) == 255)
						|| (k + 1 < dest.cols && dest.at<uchar>(j, k + 1) == 255))
						que.push(Point(k, j));
			}
		}

		while (!que.empty())
		{
			dest.at<uchar>(que.front()) = 255;
			que.pop();
		}
	}
}


void subEraseGray(Mat &src, Point point)
{
	const Point srcSize(src.cols, src.rows);
	queue<Point> que;
	que.push(point);

	while (!que.empty())
	{
		point = que.front();
		que.pop();

		if (src.at<uchar>(point) == 0)
			continue;

		src.at<uchar>(point) = 0;

		for (int i = 0; i < sizeof(directions) / sizeof(directions[0]); i++)
		{
			Point tmp1 = point + directions[i];
			Point tmp2 = srcSize - tmp1;
			if (!(tmp1.x < 0 || tmp1.y < 0 || tmp2.x <= 0 || tmp2.y <= 0))
			{
				if (src.at<uchar>(tmp1) == 255)
					que.push(tmp1);
			}
		}
	}

}


void eraseGray(const Mat& src, Mat &dest)
{
	const Point srcSize(src.cols, src.rows);
	src.copyTo(dest);
	for (int i = 0; i < dest.cols; i++)
	{
		for (int j = 0; j < dest.rows; j++)
		{
			Point point(i, j);
			if (dest.at<uchar>(point) == GRAY)
			{
				dest.at<uchar>(point) = 0;

				for (int k = 0; k < sizeof(directions) / sizeof(directions[0]); k++)
				{
					Point tmp1 = point + directions[k];
					Point tmp2 = srcSize - tmp1;
					if (!(tmp1.x < 0 || tmp1.y < 0 || tmp2.x <= 0 || tmp2.y <= 0))
					{
						if (dest.at<uchar>(tmp1) == 255)
							subEraseGray(dest, tmp1);
							
					}
				}
			}
		}
	}
}


void subIndexGray(Mat &src, Point point, int index)
{
	queue<Point> que;
	que.push(point);

	while (!que.empty())
	{
		point = que.front();
		int i = point.x, j = point.y;
		que.pop();

		if (src.at<uchar>(point) != GRAY)
			continue;

		src.at<uchar>(point) = index;

		if (i - 1 >= 0 && src.at<uchar>(point + directions[0]) == GRAY)
			que.push(Point(point + directions[0]));

		if (i + 1 < src.cols && src.at<uchar>(point + directions[1]) == GRAY)
			que.push(Point(point + directions[1]));

		if (j - 1 >= 0 && src.at<uchar>(point + directions[2]) == GRAY)
			que.push(Point(point + directions[2]));

		if (j + 1 < src.rows && src.at<uchar>(point + directions[3]) == GRAY)
			que.push(Point(point + directions[3]));
	}
}


queue<Point> indexGray(Mat& src)
{
	queue<Point> que;
	int index = 1;
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			if (src.at<uchar>(i, j) == GRAY)
			{
				que.push(Point(j, i));
				subIndexGray(src, Point(j, i), index++);
			}

			if (index == GRAY)
				return que;
		}
	}

	return que;
}


uchar subGetCrosswordBox(Mat &src, queue<Point> &directionQueue, Point direction, int index)
{
	const Point srcSize(src.cols, src.rows);
	Point point;
	map<uchar, int> map;

	while (!directionQueue.empty())
	{
		point = directionQueue.front();
		directionQueue.pop();

		Point tmp = srcSize - point;

		while (tmp.x > 0 && tmp.y > 0 && point.x >= 0 && point.y >= 0 && src.at<uchar>(point) == 255)
		{
			src.at<uchar>(point) = 50;
			point += direction;
			tmp = srcSize - point;
		}

		if (!(tmp.x > 0 && tmp.y > 0 && point.x >= 0 && point.y >= 0))
		{
			if (map.find(0) == map.end())
				map[0] = 1;
			else
				map[0]++;
			continue;
		}


		if (map.find(src.at<uchar>(point)) == map.end())
			map[src.at<uchar>(point)] = 1;
		else
			map[src.at<uchar>(point)]++;
	}

	auto i = map.begin();
	if (i == map.end())
		return 0;
	uchar maxIndex = i->first;
	int maxValue = i->second;
	for (i++; i != map.end(); i++)
	{
		if (i->second > maxValue && index != i->first)
		{
			maxIndex = i->first;
			maxValue = i->second;
		}
			
	}

	return maxIndex;
}


CrosswordBox getCrosswordBox(Mat &src, Point point)
{
	Mat copy;
	src.copyTo(copy);

	const Point srcSize(src.cols, src.rows);
	queue<Point> que;
	queue<Point> directionQueues[4];
	int indexes[4];
	int index = src.at<uchar>(point);
	que.push(point);

	while (!que.empty())
	{
		point = que.front();
		que.pop();

		if (copy.at<uchar>(point) != index)
			continue;

		for (int i = 0; i < sizeof(directions) / sizeof(directions[0]); i++)
		{
			Point tmp1 = point + directions[i];
			Point tmp2 = srcSize - tmp1;
			if (!(tmp1.x < 0 || tmp1.y < 0 || tmp2.x <= 0 || tmp2.y <= 0))
			{
				if (copy.at<uchar>(tmp1) == index)
					que.push(tmp1);
				else
					directionQueues[i].push(tmp1);
			}
		}

		copy.at<uchar>(point) = 0;
	}


	for (int i = 0; i < sizeof(directionQueues) / sizeof(directionQueues[0]); i++)
	{
		src.copyTo(copy);
		indexes[i] = subGetCrosswordBox(copy, directionQueues[i], directions[i], index);
	}

	return{ index, indexes[0], indexes[1], indexes[2], indexes[3], (int)0x80000000 };
}


void setXYinCrosswordBoxes(CrosswordBox *crosswordBoxes, int size)
{
	queue<CrosswordBox*> que;
	que.push(crosswordBoxes);
	crosswordBoxes->point = Point(0, 0);
	while (!que.empty())
	{
		CrosswordBox *current = que.front();
		que.pop();

		for (int i = 0; i < sizeof(CrosswordBox::neighborhoodIds) / sizeof(CrosswordBox::neighborhoodIds[0]); i++)
		{
			CrosswordBox *neighborhood = &crosswordBoxes[current->neighborhoodIds[i] - 1];
			if (current->neighborhoodIds[i] != 0 && neighborhood->point.x == 0x80000000)
			{
				neighborhood->point = current->point + directions[i];
				que.push(neighborhood);
			}
		}

	}
}


Point getMatrixSize(CrosswordBox *crosswordBoxes, int n)
{
	int minY = crosswordBoxes->point.y,
		maxY = crosswordBoxes->point.y,
		minX = crosswordBoxes->point.x,
		maxX = crosswordBoxes->point.x;

	for (int i = 1; i < n; i++)
	{
		if (crosswordBoxes[i].point.x == 0x80000000)
			continue;

		if (maxY < crosswordBoxes[i].point.y)
			maxY = crosswordBoxes[i].point.y;

		else if (minY > crosswordBoxes[i].point.y)
			minY = crosswordBoxes[i].point.y;

		if (maxX < crosswordBoxes[i].point.x)
			maxX = crosswordBoxes[i].point.x;

		else if (minX > crosswordBoxes[i].point.x)
			minX = crosswordBoxes[i].point.x;
	}

	Point size(maxX - minX + 1, maxY - minY + 1);
	Point translation(size.x - maxX - 1, size.y - maxY - 1);

	if (translation == Point(0, 0))
		return size;

	for (int i = 0; i < n; i++)
	{
		if (crosswordBoxes[i].point.x == 0x80000000)
			continue;

		crosswordBoxes[i].point += translation;
	}

	return size;
}


void frameGray(const Mat &src, Mat &dest)
{
	src.copyTo(dest);
	for (int i = 0; i < dest.rows; i++)
	{
		for (int j = 0; j < dest.cols; j++)
		{
			if (dest.at<uchar>(i, j) == 255
				&& i - 1 >= 0
				&& (dest.at<uchar>(i - 1, j) == 255 || dest.at<uchar>(i - 1, j) == GRAY)
				&& i + 1 < dest.rows
				&& (dest.at<uchar>(i + 1, j) == 255 || dest.at<uchar>(i + 1, j) == GRAY)
				&& j - 1 >= 0
				&& (dest.at<uchar>(i, j - 1) == 255 || dest.at<uchar>(i, j - 1) == GRAY)
				&& j + 1 < dest.cols
				&& (dest.at<uchar>(i, j + 1) == 255 || dest.at<uchar>(i, j + 1) == GRAY))
				dest.at<uchar>(i, j) = GRAY;
		}
	}
}