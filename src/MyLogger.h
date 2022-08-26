//
// Created by 黄铮 on 2022/4/8.
//

#ifndef TRAFFICSCORE_TOOLS_MYLOGGER_H_
#define TRAFFICSCORE_TOOLS_MYLOGGER_H_

#include "iostream"
#include "MyMacros.h"

class MyLogger
{
 public:
	~MyLogger()= default;
	MyLogger(const MyLogger&)=delete;
	MyLogger& operator=(const MyLogger&)=delete;
	static log4cplus::Logger& getInstance(){
		static MyLogger instance;
		return instance.logger;

	}
 private:
	log4cplus::Logger logger = MYLOG_DEFINE("MyLogger");
	MyLogger() = default;
};

#endif //TRAFFICSCORE_TOOLS_MYLOGGER_H_
