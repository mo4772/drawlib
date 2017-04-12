#ifndef LOG_H
#define LOG_H


#ifdef WIN32
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include "glog/logging.h"
#else
#include <glog/logging.h>
#endif

#define LOG_INFO LOG(INFO) 
#define LOG_DEBUG LOG(INFO)
#define LOG_ERROR LOG(ERROR)

#endif