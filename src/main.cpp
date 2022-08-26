#include <iostream>
#include "MyLogger.h"
#include "core/SpatialHelper.h"
#include "service/RestService.h"

using namespace std;

#if 1
int main() {
    GDALAllRegister();

    log4cplus::Initializer initializer;
    log4cplus::PropertyConfigurator pc(LOG4CPLUS_TEXT("log4cplus.properties"));
    pc.configure();

    RestService::start();

    cout << "service stopped" << endl;
    return 0;
}
#endif