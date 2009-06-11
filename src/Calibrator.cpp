/*
 * Calibrator.cpp
 *
 *  Created on: 5-Jun-2009
 *      Author: loyola
 */

#include "Calibrator.h"
#include "BarcodeInfo.h"
#include "BinRegion.h"
#include "Dib.h"
#include "UaLogger.h"
#include "UaAssert.h"
#include "Util.h"

Calibrator::Calibrator() {
}

Calibrator::~Calibrator() {
	BinRegion * c;

	while (rowBinRegions.size() > 0) {
		c = rowBinRegions.back();
		rowBinRegions.pop_back();
		delete c;
	}
	while (colBinRegions.size() > 0) {
		c = colBinRegions.back();
		colBinRegions.pop_back();
		delete c;
	}
}

void Calibrator::processImage(Dib & dib) {
	Decoder::processImage(dib, msgInfos);
	width = dib.getWidth();
	height = dib.getHeight();
	sortRegions();
}

void Calibrator::processImage(DmtxImage & image) {
	Decoder::processImage(image, msgInfos);
	width = dmtxImageGetProp(&image, DmtxPropWidth);
	height = dmtxImageGetProp(&image, DmtxPropHeight);
	sortRegions();
}

/* Finds rows and columns by examining each decode region's top left corner.
 * Each region is assigned to a row and column.
 *
 * Once rows and columns are determined, they are sorted. Once this is done,
 * the regions can then be sorted according to row and column.
 */
void Calibrator::sortRegions() {
	bool insideRowBin;
	bool insideColBin;

	for (unsigned i = 0, n = msgInfos.size(); i < n; ++i) {
		insideRowBin = false;
		insideColBin = false;

		DmtxPixelLoc & tlCorner = msgInfos[i]->getTopLeftCorner();
		DmtxPixelLoc & brCorner = msgInfos[i]->getBotRightCorner();

		UA_DOUT(1, 9, "tag " << i << " : tlCorner/" << tlCorner.X << "," << tlCorner.Y
				<< "  brCorner/" << brCorner.X << "," << brCorner.Y);

		for (unsigned c = 0, cn = colBinRegions.size(); c < cn; ++c) {
			BinRegion & bin = *colBinRegions[c];

			int lDiff = tlCorner.X - bin.getMin();
			int rDiff = brCorner.X - bin.getMax();

			UA_DOUT(1, 9, "col " << c << ": left_diff/" << lDiff << ": right_diff/" << rDiff);

			if ((lDiff >= 0) && (rDiff <= 0)) {
				insideColBin = true;
				msgInfos[i]->setColBinRegion(&bin);
			}
			else if ((lDiff < 0) && (lDiff > -BIN_THRESH)) {
				insideColBin = true;
				msgInfos[i]->setColBinRegion(&bin);
				bin.setMin(tlCorner.X);
				UA_DOUT(1, 9, "col update min " << bin.getMin());
			}
			else if ((rDiff > 0) && (rDiff < BIN_THRESH)) {
				insideColBin = true;
				msgInfos[i]->setColBinRegion(&bin);
				bin.setMax(brCorner.X);
				UA_DOUT(1, 9, "col update max " << bin.getMax());
			}
		}

		for (unsigned r = 0, rn = rowBinRegions.size(); r < rn; ++r) {
			BinRegion & bin = *rowBinRegions[r];

			int tDiff = tlCorner.Y - bin.getMin();
			int bDiff = brCorner.Y - bin.getMax();

			UA_DOUT(1, 9, "row " << r << ": top_diff/" << tDiff << ": bot_diff/" << bDiff);

			if ((tDiff >= 0) && (bDiff <= 0)) {
				insideRowBin = true;
				msgInfos[i]->setRowBinRegion(&bin);
			}
			else if ((tDiff < 0) && (tDiff > -BIN_THRESH)) {
				insideRowBin = true;
				msgInfos[i]->setRowBinRegion(&bin);
				bin.setMin(tlCorner.Y);
				UA_DOUT(1, 9, "row update min " << bin.getMin());
			}
			else if ((bDiff > 0) && (bDiff < BIN_THRESH)) {
				insideRowBin = true;
				msgInfos[i]->setRowBinRegion(&bin);
				bin.setMax(brCorner.Y);
				UA_DOUT(1, 9, "row update max " << bin.getMax());
			}
		}

		if (!insideColBin) {
			BinRegion * newBinRegion = new BinRegion(BinRegion::ORIENTATION_VER,
					(unsigned) tlCorner.X, (unsigned) brCorner.X);
			UA_ASSERT_NOT_NULL(newBinRegion);
			UA_DOUT(1, 9, "new col " << colBinRegions.size() << ": " << *newBinRegion);
			colBinRegions.push_back(newBinRegion);
			msgInfos[i]->setColBinRegion(newBinRegion);
		}

		if (!insideRowBin) {
			BinRegion * newBinRegion = new BinRegion(BinRegion::ORIENTATION_HOR,
					(unsigned) tlCorner.Y, (unsigned) brCorner.Y);
			UA_ASSERT_NOT_NULL(newBinRegion);
			UA_DOUT(1, 9, "new row " << rowBinRegions.size() << ": " << *newBinRegion);
			rowBinRegions.push_back(newBinRegion);
			msgInfos[i]->setRowBinRegion(newBinRegion);
		}
	}

	sort(rowBinRegions.begin(), rowBinRegions.end(), BinRegionSort());
	sort(colBinRegions.begin(), colBinRegions.end(), BinRegionSort());

	// assign ranks now and add threshold
	for (unsigned i = 0, n = colBinRegions.size(); i < n; ++ i) {
		BinRegion & c = *colBinRegions[i];

		unsigned min = c.getMin();
		c.setMin(min > 15 ? min - 15 : 0);

		unsigned max = c.getMax();
		c.setMax(max < width - 15 ? max + 15 : width);

		c.setRank(i);
		UA_DOUT(1, 5, "col BinRegion " << i << ": " << c);
	}
	for (unsigned i = 0, n = rowBinRegions.size(); i < n; ++ i) {
		BinRegion & c = *rowBinRegions[i];

		unsigned min = c.getMin();
		c.setMin(min > 15 ? min - 15 : 0);

		unsigned max = c.getMax();
		c.setMax(max < height - 15 ? max + 15 : height);

		c.setRank(i);
		UA_DOUT(1, 5, "row BinRegion " << i << ": " << c);
	}

	UA_DOUT(1, 3, "number of columns: " << colBinRegions.size());
	UA_DOUT(1, 3, "number of rows: " << rowBinRegions.size());

	sort(msgInfos.begin(), msgInfos.end(), BarcodeInfoSort());
}

void Calibrator::saveRegionsToIni(unsigned plateNum, CSimpleIniA & ini) {
	UA_ASSERT(msgInfos.size() > 0);

	SI_Error rc;
	string secName = "plate-" + to_string(plateNum) + "-" + INI_SECTION_NAME;

	unsigned maxCol = msgInfos[0]->getColBinRegion().getRank();
	ostringstream key, value;
	for (int r = rowBinRegions.size() - 1; r >= 0; --r) {
		for (unsigned c = 0, cn = colBinRegions.size(); c < cn; ++c) {
			key.str("");
			value.str("");

			key << INI_REGION_LABEL << rowBinRegions[r]->getRank() << "_"
				<< maxCol - colBinRegions[c]->getRank();
			value << colBinRegions[c]->getMin() << ","
			      << rowBinRegions[r]->getMin() << ","
			      << colBinRegions[c]->getMax() << ","
			      << rowBinRegions[r]->getMax();

			rc = ini.SetValue(secName.c_str(), key.str().c_str(), value.str().c_str());
			UA_ASSERT(rc >= 0);
		}
	}
}

void Calibrator::imageShowBins(Dib & dib, RgbQuad & quad) {
	UA_DOUT(1, 3, "marking tags ");
	for (unsigned c = 0, cn = colBinRegions.size(); c < cn; ++c) {
		UA_DOUT(1, 3, "line (" << colBinRegions[c]->getMin() << ", 0) ("
				 << colBinRegions[c]->getMin() << ", "
				 << dib.getHeight()-1 << ")");
		UA_DOUT(1, 3, "line (" << colBinRegions[c]->getMax() << ", 0) ("
				 << colBinRegions[c]->getMax() << ", "
				 << dib.getHeight()-1 << ")");

		dib.line(colBinRegions[c]->getMin(), 0,
				 colBinRegions[c]->getMin(), dib.getHeight()-1, quad);
		dib.line(colBinRegions[c]->getMax(), 0,
				 colBinRegions[c]->getMax(), dib.getHeight()-1, quad);
	}
	for (unsigned r = 0, rn = rowBinRegions.size(); r < rn; ++r) {
		UA_DOUT(1, 3, "line (0, " << rowBinRegions[r]->getMin() << ") ("
				 << dib.getWidth()-1 << ", "
				 << ", " << rowBinRegions[r]->getMin() << ")");
		UA_DOUT(1, 3, "line (0, " << rowBinRegions[r]->getMax() << ") ("
				 << dib.getWidth()-1 << ", "
				 << ", " << rowBinRegions[r]->getMax() << ")");

		dib.line(0, rowBinRegions[r]->getMin(),
				 dib.getWidth()-1, rowBinRegions[r]->getMin(), quad);
		dib.line(0, rowBinRegions[r]->getMax(),
				 dib.getWidth()-1, rowBinRegions[r]->getMax(), quad);

	}
}
