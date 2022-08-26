//#define CATCH_CONFIG_MAIN
//
//#include "catch.hpp"
//#include <ogr_geometry.h>
//#include <gdal_priv.h>
//#include "../MyLogger.h"
//#include "../core/SpatialHelper.h"
//#include "iostream"
//#include "vector"
//
//using namespace std;
//
//static Field_Attr fieldAttr("OBJECTID", "EXP_NO", "S_POINT", "E_POINT", "S_H", "E_H", "D_S");
//static Data_Attr dataAttr(R"(E:\testdata\pipeline\wxJSPipe.gdb)", "JSPOINT", "JSLINE",
//                          R"(E:\testdata\pipeline\wxWSPipe.gdb)", "WSPOINT", "WSLINE");
//
//SpatialHelper *spatial_helper = nullptr;
//
//void init() {
//    GDALAllRegister();
//
//    vector<long> objList;
//    objList.push_back(25148);
//    objList.push_back(25136);
//
//    vector<long> objList_other;
//    objList_other.push_back(10621);
//    objList_other.push_back(10623);
//    objList_other.push_back(10624);
//
//    spatial_helper = new SpatialHelper(fieldAttr, dataAttr);
//    spatial_helper->setObjectIdList(objList);
//    spatial_helper->setObjectIdOtherList(objList_other);
//
//}
//
//double cal_point_on_line_altitude_test1() {
//    Point_Attr start_point = {0, 0, 10};
//    Point_Attr end_point = {10, 10, 20};
//    Point_Attr current_point = {2, 2, -1};
//    spatial_helper->cal_point_on_line_altitude(start_point, end_point, current_point);
////    cout << "current_point.altitude = " << current_point.altitude << endl;
//    return current_point.altitude;
//}
//
//double cal_point_on_line_altitude_test2() {
//    Point_Attr start_point = {10, 10, 20};
//    Point_Attr end_point = {0, 0, 10};
//    Point_Attr current_point = {2, 2, -1};
//    spatial_helper->cal_point_on_line_altitude(start_point, end_point, current_point);
////    cout << "current_point.altitude = " << current_point.altitude << endl;
//    return current_point.altitude;
//}
//
////bool cal_horizontal_spacing() {
////    spatial_helper->horizontal_spacing();
////    return true;
////}
////
////bool cal_vertical_spacing() {
////    spatial_helper->vertical_spacing();
////    return true;
////}
////
////bool cal_horizontal_and_vertical_spacing() {
////    spatial_helper->horizontal_and_vertical_spacing();
////    return true;
////}
//
//TEST_CASE("coreTest", "[test]")
//{
//    init();
//
//    SECTION("get_line_from_geometry_test")
//    {
//        OGRLineString *line = OGRGeometryFactory::createGeometry(OGRwkbGeometryType::wkbLineString)->toLineString();
//        line->addPoint(0, 0);
//        line->addPoint(1, 1);
//        line->addPoint(2, 2);
//
//        OGRLineString *lineNew = nullptr;
//        SpatialHelper::get_line_from_geometry(line, &lineNew);
//        bool ret = lineNew != nullptr;
//
//        OGRGeometryFactory::destroyGeometry(line);
//
//        REQUIRE(ret);
//    }
//
//    SECTION("cal_point_on_line_altitude")
//    {
//        REQUIRE(cal_point_on_line_altitude_test1() == cal_point_on_line_altitude_test2());
//    }
//
//    SECTION("get_lines_intersection_point_test")
//    {
//        auto *pLine1 = new OGRLineString();
//        pLine1->setPoint(0, new OGRPoint(0, 0));
//        pLine1->setPoint(1, new OGRPoint(10.0, 0));
//
//        auto *pLine2 = new OGRLineString();
//        pLine2->setPoint(0, new OGRPoint(3.0, 10));
//        pLine2->setPoint(1, new OGRPoint(3.0, -10));
//
//        OGRPoint *point = nullptr;
//        spatial_helper->get_lines_intersection_point(pLine1, pLine2, &point);
//
//
//        REQUIRE((point->getX() == 3.0 && point->getY() == 0));
//    }
//
//
////    SECTION("cal_horizontal_spacing_test")
////    {
////        cal_horizontal_spacing();
////        REQUIRE(!spatial_helper->getResult().empty());
////    }
////
////    SECTION("cal_vertical_spacing_test")
////    {
////        cal_vertical_spacing();
////        REQUIRE(!spatial_helper->getResult().empty());
////    }
////
////    SECTION("cal_horizontal_and_vertical_spacing_test")
////    {
////        cal_horizontal_and_vertical_spacing();
////        REQUIRE(!spatial_helper->getResult().empty());
////    }
//
//    delete spatial_helper;
//}
//
//#if 0
//
//int main() {
//    // 初始化日志模块
//    log4cplus::Initializer initializer;
//    log4cplus::PropertyConfigurator pc(LOG4CPLUS_TEXT("log4cplus.properties"));
//    pc.configure();
//
//    init();
//
//
//    cout << spatial_helper->getResult() << endl;
////    cal_horizontal_spacing();
////    cal_vertical_spacing();
//    cal_horizontal_and_vertical_spacing();
//
//    cout << spatial_helper->getResult() << endl;
//
//    delete spatial_helper;
//
//    cout << ">>>>> finish <<<<<<<<" << endl;
//    return 0;
//}
//
//#endif