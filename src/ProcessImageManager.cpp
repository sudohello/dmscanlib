/*
 Dmscanlib is a software library and standalone application that scans
 and decodes libdmtx compatible test-tubes. It is currently designed
 to decode 12x8 pallets that use 2D data-matrix laser etched test-tubes.
 Copyright (C) 2010 Canadian Biosample Repository

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "ProcessImageManager.h"
#include "BarcodeThread.h"
#include "UaLogger.h"
#include "UaAssert.h"
#include "Dib.h"
#include "Decoder.h"
#include "BarcodeInfo.h"
#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#define SLEEP Sleep
#else 
#define SLEEP sleep
#endif

#include <map>

ProcessImageManager::ProcessImageManager(Decoder * decoder, double scanGap,
		unsigned squareDev, unsigned edgeThresh, unsigned corrections) {
	this->scanGap = scanGap;
	this->squareDev = squareDev;
	this->edgeThresh = edgeThresh;
	this->corrections = corrections;
	this->decoder = decoder;
}

ProcessImageManager::~ProcessImageManager() {
	for (unsigned i = 0, n = allThreads.size(); i < n; ++i) {


		delete allThreads[i];
	}
}

//inclusive first exclusive last
void ProcessImageManager::threadProcessRange(vector<BarcodeThread *> & threads,
		unsigned int _first, unsigned int _last) {

	time_t timeStart, timeEnd;

	unsigned int first, last;

	if (_last < _first) {
		first = _last;
		last = _first;
		UA_DOUT(3, 1, "WARNING: range is switched in ProcessImageManager.");
	} else {
		first = _first;
		last = _last;
	}

	for (unsigned int i = first; i < last; i++)
		threads[i]->start();

	time(&timeStart);

	while (true) {
		bool done = true;
		for (unsigned int j = first; j < last; j++)
			if (!threads[j]->isFinished())
				done = false;

		if (done)
			break;

		time(&timeEnd);
		if (difftime(timeEnd, timeStart) >= JOIN_TIMEOUT_SEC) {
			UA_DOUT(3, 1, "Error:: Some threads have timed out.");
			break;
		}

		usleep(500);
	}

}

void ProcessImageManager::threadHandler(vector<BarcodeThread *> & threads) {

	unsigned int threadCount =
			(threads.size() < THREAD_NUM ) ? threads.size() : THREAD_NUM;

	unsigned int remainder = ((threads.size()) % threadCount);

	printf("remainder %d\n", remainder);

	for (unsigned int i = 0; i < threads.size() - remainder; i += threadCount) {
		UA_DOUT(
				3,
				5,
				"Threads for barcodes finished: "<<i << "/" << i + threadCount-1);
		threadProcessRange(threads, i, i + threadCount);

	}

	if (remainder > 0) {
		UA_DOUT(
				3,
				5,
				"Threads for barcodes finished(remainder): " << threads.size() - remainder << "/" << threads.size() - remainder-1 << " finished.");

		threadProcessRange(threads, threads.size() - remainder, threads.size());
	}

	printf("done all\n");

}

void ProcessImageManager::generateBarcodes(Dib * dib,
		vector<vector<BarcodeInfo *> > & barcodeInfos) {
	vector<BarcodeThread *> threads;

	for (unsigned row = 0, rows = barcodeInfos.size(); row < rows; ++row) {
		for (unsigned col = 0, cols = barcodeInfos[row].size(); col < cols;
				++col) {

			BarcodeInfo * info = barcodeInfos[row][col];
			if (info == NULL
			)
				continue;

			CvRect & rect = info->getPreProcessBoundingBox();

			Dib * croppedDib = Dib::crop(*dib, rect.x, rect.y,
					rect.x + rect.width, rect.y + rect.height);

			BarcodeThread * thread = new BarcodeThread(this, scanGap, squareDev,
					edgeThresh, corrections, rect, croppedDib, *barcodeInfos[row][col]);

			allThreads.push_back(thread);
			threads.push_back(thread);
		}
	}
	threadHandler(threads);

}
