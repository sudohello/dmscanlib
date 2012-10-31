/*
 * DecodeOptions.cpp
 *
 *  Created on: 2012-10-11
 *      Author: nelson
 */

#include "DecodeOptions.h"
#include <stddef.h>
#include <jni.h>

namespace dmscanlib {

DecodeOptions::DecodeOptions(double scanGap, long squareDev,
		long edgeThresh, long corrections, long shrink, double cellDistance) {

    this->scanGap       = scanGap;
    this->squareDev     = squareDev;
    this->edgeThresh    = edgeThresh;
    this->corrections   = corrections;
    this->shrink        = shrink;
    this->cellDistance  = cellDistance;
}

DecodeOptions::DecodeOptions(JNIEnv *env, jobject decodeOptionsObj) {
    jclass decodeOptionsJavaClass = env->GetObjectClass(decodeOptionsObj);

    // run the following command to obtain method signatures from a class.
    // javap -s -p edu.ualberta.med.scannerconfig.dmscanlib.ScanRegion
    jmethodID getMethod = env->GetMethodID(
    		decodeOptionsJavaClass, "getScanGap", "()D");
    if(env->ExceptionOccurred()) {
    	return;
    }
    scanGap= env->CallDoubleMethod(decodeOptionsObj, getMethod, NULL);

    getMethod = env->GetMethodID(decodeOptionsJavaClass, "getSquareDev", "()J");
    if(env->ExceptionOccurred()) {
    	return;
    }
    squareDev= env->CallLongMethod(decodeOptionsObj, getMethod, NULL);

    getMethod = env->GetMethodID(decodeOptionsJavaClass, "getEdgeThresh", "()J");
    if(env->ExceptionOccurred()) {
    	return;
    }
    edgeThresh= env->CallLongMethod(decodeOptionsObj, getMethod, NULL);

    getMethod = env->GetMethodID(decodeOptionsJavaClass, "getCorrections", "()J");
    if(env->ExceptionOccurred()) {
    	return;
    }
    corrections= env->CallLongMethod(decodeOptionsObj, getMethod, NULL);

    getMethod = env->GetMethodID(decodeOptionsJavaClass, "getShrink", "()J");
    if(env->ExceptionOccurred()) {
    	return;
    }
    shrink = env->CallLongMethod(decodeOptionsObj, getMethod, NULL);

    getMethod = env->GetMethodID(decodeOptionsJavaClass, "getCellDistance", "()D");
    if(env->ExceptionOccurred()) {
    	return;
    }
    cellDistance= env->CallDoubleMethod(decodeOptionsObj, getMethod, NULL);

}

DecodeOptions::~DecodeOptions() {
}

std::ostream & operator<<(std::ostream &os, const DecodeOptions & m) {
	os << " scanGap/" << m.scanGap
			<< " squareDev/" << m.squareDev
			<< " edgeThresh/" << m.edgeThresh
			<< " corrections/"	<< m.corrections
			<< " shrink/" << m.shrink
			<< " cellDistance/" << m.cellDistance;
	return os;
}

} /* namespace */
