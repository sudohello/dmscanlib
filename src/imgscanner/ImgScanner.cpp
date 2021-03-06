#define _CRT_SECURE_NO_DEPRECATE

#include "imgscanner/ImgScanner.h"
#include "imgscanner/ImgScannerTwain.h"
#include "imgscanner/ImgScannerSimulator.h"

#include <memory>

namespace dmscanlib {

ImgScanner::ImgScanner() {

}

ImgScanner::~ImgScanner() {

}

std::unique_ptr<ImgScanner> ImgScanner::create() {
#ifdef WIN32
    return std::unique_ptr<ImgScanner>(new imgscanner::ImgScannerTwain());
#else
    return std::unique_ptr<ImgScanner>(new imgscanner::ImgScannerSimulator());
#endif
}

} /* namespace */
