#include <gdal.h>
#include <gdal_priv.h>
#include <ogr_core.h>
#include <ogr_geometry.h>
#include "ogrsf_frmts.h"
#include <utility>
#include <fmt/format.h>
#include <set>
#include <iomanip>
#include "SpatialHelper.h"
#include "../StringUtil.hpp"
#include "../NumberUtil.hpp"
#include "../MyLogger.h"


SpatialHelper::SpatialHelper(Field_Attr &field_attr, Data_Attr &data_attr) : field_attr_(field_attr),
                                                                             data_attr_(data_attr) {
    dataset_ = static_cast<GDALDataset *>(
            GDALOpenEx(data_attr_.data_path.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
    if (dataset_ == nullptr) {
        LOG4CPLUS_ERROR(MyLogger::getInstance(), "GDALOpen fail: " << data_attr_.data_path);
        return;
    }

    if (data_attr_.data_path == data_attr_.data_path_other) {
        dataset_other_ = dataset_;
    } else {
        dataset_other_ = static_cast<GDALDataset *>(
                GDALOpenEx(data_attr_.data_path_other.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
        if (dataset_other_ == nullptr) {
            LOG4CPLUS_ERROR(MyLogger::getInstance(), "GDALOpen fail: " << data_attr_.data_path_other);
            return;
        }
    }
}

SpatialHelper::~SpatialHelper() {
    if (dataset_ != nullptr)
        GDALClose(dataset_);

    if (dataset_other_ != nullptr)
        GDALClose(dataset_other_);
}

void SpatialHelper::setObjectIdList(const vector<long> &objectIdList) {
//    object_id_list_.assign(objectIdList.begin(), objectIdList.end());
    object_id_list_ = objectIdList;
}

void SpatialHelper::setObjectIdOtherList(const vector<long> &objectIdOtherList) {
//    object_id_other_list_.assign(objectIdOtherList.begin(), objectIdOtherList.end());
    object_id_other_list_ = objectIdOtherList;
}

void SpatialHelper::insertResult(const string &key_name, const string &key_val, double distance) {
    if (!result_.contains(key_val)) {
        result_[key_val] = {
                {key_name, distance}
        };
    } else {
        nlohmann::json res = result_[key_val].get<nlohmann::json>();
        res[key_name] = distance;
        result_[key_val] = res;
    }
}


bool SpatialHelper::init_lines(std::map<long, OGRGeometry *> &geometry_map,
                               std::map<long, OGRGeometry *> &geometry_other_map,
                               std::map<long, Line_Attr> &line_attr_map,
                               std::map<long, Line_Attr> &line_attr_other_map,
                               bool is_get_point) {
    do_get_lines_attr(dataset_,
                      data_attr_.point_layer_name,
                      data_attr_.line_layer_name,
                      object_id_list_,
                      line_attr_map,
                      geometry_map,
                      is_get_point);

    do_get_lines_attr(dataset_other_,
                      data_attr_.point_layer_name_other,
                      data_attr_.line_layer_name_other,
                      object_id_other_list_,
                      line_attr_other_map,
                      geometry_other_map,
                      is_get_point);

    if (line_attr_map.empty()) {
        LOG4CPLUS_ERROR(MyLogger::getInstance(), "图层实体为空! layer=" << data_attr_.line_layer_name);
        return false;
    }

    if (line_attr_other_map.empty()) {
        LOG4CPLUS_ERROR(MyLogger::getInstance(), "图层实体为空! layer=" << data_attr_.point_layer_name_other);
        return false;
    }

    return true;
}

void SpatialHelper::run(do_distance_type& dis_type) {
    if (!init_succeed()) {
        return;
    }

    std::map<long, OGRGeometry *> geometry_map;
    std::map<long, OGRGeometry *> geometry_map_other;

    std::map<long, Line_Attr> line_attr_map;
    std::map<long, Line_Attr> line_attr_other_map;

    do_get_lines_attr(dataset_, data_attr_.point_layer_name, data_attr_.line_layer_name, object_id_list_, line_attr_map, geometry_map);
    do_get_lines_attr(dataset_other_, data_attr_.point_layer_name_other, data_attr_.line_layer_name_other, object_id_other_list_, line_attr_other_map, geometry_map_other);

    switch (dis_type) {
        case horizontal:
            do_horizontal_spacing(geometry_map, geometry_map_other, line_attr_map, line_attr_other_map);
            break;
        case vertical:
            do_vertical_spacing(geometry_map, geometry_map_other, line_attr_map, line_attr_other_map);
            break;
        case both:
            do_horizontal_spacing(geometry_map, geometry_map_other, line_attr_map, line_attr_other_map);
            do_vertical_spacing(geometry_map, geometry_map_other, line_attr_map, line_attr_other_map);
            break;
        default:
            do_horizontal_spacing(geometry_map, geometry_map_other, line_attr_map, line_attr_other_map);
            do_vertical_spacing(geometry_map, geometry_map_other, line_attr_map, line_attr_other_map);
    }

    releaseGeometries(geometry_map);
    releaseGeometries(geometry_map_other);
}

void SpatialHelper::do_horizontal_spacing(std::map<long, OGRGeometry *> &geometry_map,
                                          std::map<long, OGRGeometry *> &geometry_map_other,
                                          std::map<long, Line_Attr>& line_attr_map,
                                          std::map<long, Line_Attr>& line_attr_other_map) {


    if (!init_lines(geometry_map,geometry_map_other,line_attr_map, line_attr_other_map, false)) {
        return;
    }

    // 计算
    for (auto geom_pair: geometry_map) {
        Line_Attr &attr = line_attr_map.find(geom_pair.first)->second;
        for (auto geom_pair_other: geometry_map_other) {
            Line_Attr &attr_other = line_attr_other_map.find(geom_pair_other.first)->second;
            double distance = cal_horizontal_spacing_between_two(geom_pair.second, geom_pair_other.second);
            // 减掉2个管径
            distance = distance - attr.pipe_diameter / 2 - attr_other.pipe_diameter / 2;
            distance = distance < 0 ? 0 : distance;

            hzNumberUtil::keepDecimal(distance, 2);

            // id1_id2 , 类似  1001_16542
            string key_val = fmt::format("{}_{}", geom_pair.first, geom_pair_other.first);
            insertResult("horizontal", key_val, distance);
        }
    }
}

void SpatialHelper::do_vertical_spacing(std::map<long, OGRGeometry *> &geometry_map,
                                        std::map<long, OGRGeometry *> &geometry_map_other,
                                        std::map<long, Line_Attr>& line_attr_map,
                                        std::map<long, Line_Attr>& line_attr_other_map) {
    if (!init_lines(geometry_map,geometry_map_other,line_attr_map, line_attr_other_map)) {
        return;
    }

    // 计算
    for (auto geom_pair: geometry_map) {
        Line_Attr &attr = line_attr_map.find(geom_pair.first)->second;
        for (auto geom_pair_other: geometry_map_other) {
            Line_Attr &attr_other = line_attr_other_map.find(geom_pair_other.first)->second;
            double distance = cal_vertical_spacing_between_two(geom_pair.second,
                                                               attr,
                                                               geom_pair_other.second,
                                                               attr_other);
            // 减掉2个管径
            distance = distance - attr.pipe_diameter / 2 - attr_other.pipe_diameter / 2;
            distance = distance < 0 ? 0 : distance;

            hzNumberUtil::keepDecimal(distance, 2);
            // id1_id2 , 类似  1001_16542
            string key_val = fmt::format("{}_{}", geom_pair.first, geom_pair_other.first);
            insertResult("vertical", key_val, distance);
        }
    }
}

void SpatialHelper::get_line_from_geometry(OGRGeometry *geom, OGRLineString **line) {
    if (geom == nullptr) {
        return;
    }

    if (geom->getGeometryType() == wkbLineString) {
        *line = geom->toLineString();
    } else if (geom->getGeometryType() == wkbMultiLineString) {
        OGRMultiLineString *multiLine = geom->toMultiLineString();
        *line = (OGRLineString *) multiLine->getGeometryRef(0);
    }
}

void SpatialHelper::releaseGeometries(std::map<long, OGRGeometry *> &geometry_map) {
    for (auto geo_pair: geometry_map) {
        if (geo_pair.second->IsValid())
            OGRGeometryFactory::destroyGeometry(geo_pair.second);
    }
}

bool SpatialHelper::init_succeed() {
    if (dataset_ == nullptr) {
        LOG4CPLUS_ERROR(MyLogger::getInstance(), "无法打开文件, file= " << data_attr_.data_path);
        return false;
    }

    if (dataset_other_ == nullptr) {
        LOG4CPLUS_ERROR(MyLogger::getInstance(), "无法打开文件, file= " << data_attr_.data_path_other);
        return false;
    }

    if (object_id_list_.empty() || object_id_other_list_.empty()) {
        LOG4CPLUS_ERROR(MyLogger::getInstance(), "必须设置要运算的objectid列表");
        return false;
    }

    LOG4CPLUS_INFO(MyLogger::getInstance(), "初始化运行数据成功");

    return true;
}

double SpatialHelper::cal_horizontal_spacing_between_two(OGRGeometry *geom, OGRGeometry *geom_other) {
    OGRLineString *line = nullptr;
    get_line_from_geometry(geom, &line);

    OGRLineString *line_other = nullptr;
    get_line_from_geometry(geom_other, &line_other);

    if (line != nullptr && line_other != nullptr) {
        return line->Distance(line_other);
    }

    return 0;
}

double SpatialHelper::cal_vertical_spacing_between_two(OGRGeometry *geom, Line_Attr &line_attr,
                                                       OGRGeometry *geom_other, Line_Attr &line_attr_other) {
    if (geom == nullptr || geom_other == nullptr) {
        LOG4CPLUS_ERROR(MyLogger::getInstance(), "cal_vertical_spacing_between_two, 有一个无效实体");
        return 0;
    }

    OGRPoint *point = nullptr;
    get_lines_intersection_point(geom, geom_other, &point);
    // 是否有交点
    if (point == nullptr || !point->IsValid()) {
        return 0;
    }

    // 获取交点在第1条线上的高程
    Point_Attr intersection_point{};
    get_intersection_point_altitude(line_attr, point, intersection_point);

    // 获取交点在第2条线上的高程
    Point_Attr intersection_point_other{};
    get_intersection_point_altitude(line_attr_other, point, intersection_point_other);

    // 释放交点
    OGRGeometryFactory::destroyGeometry(point);

    // 垂直距离就是2个交点的高程差
    return abs(intersection_point.altitude - intersection_point_other.altitude);
}

void SpatialHelper::get_intersection_point_altitude(const Line_Attr &line_attr,
                                                    const OGRPoint *point,
                                                    Point_Attr &intersection_point) {
    Point_Attr start_point{};
    start_point.x = line_attr.start_point.x;
    start_point.y = line_attr.start_point.y;
    start_point.altitude = line_attr.start_altitude;

    Point_Attr end_point{};
    end_point.x = line_attr.end_point.x;
    end_point.y = line_attr.end_point.y;
    end_point.altitude = line_attr.end_altitude;

    intersection_point.x = point->getX();
    intersection_point.y = point->getY();
    cal_point_on_line_altitude(start_point, end_point, intersection_point);
}

void SpatialHelper::get_lines_intersection_point(struct OGRGeometry *geom,
                                                 struct OGRGeometry *geom_other,
                                                 OGRPoint **point) {
    OGRLineString *line = nullptr;
    get_line_from_geometry(geom, &line);

    OGRLineString *line_other = nullptr;
    get_line_from_geometry(geom_other, &line_other);

    // 求交点
    OGRGeometry *pGeometry = line->Intersection(line_other);
    if (pGeometry->IsValid() && (pGeometry->getGeometryType() == OGRwkbGeometryType::wkbPoint)) {
        *point = pGeometry->toPoint();
    }
}

void SpatialHelper::cal_point_on_line_altitude(const Point_Attr &start_point,
                                               const Point_Attr &end_point,
                                               Point_Attr &intersection_point) {
    double x_len = abs(start_point.x - end_point.x);
    double cal_start_altitude;
    double a;

    // 把点低的看做起点来算
    if (start_point.y <= end_point.y) {
        a = abs(intersection_point.x - start_point.x);
        cal_start_altitude = start_point.altitude;
    } else {
        a = abs(intersection_point.x - end_point.x);
        cal_start_altitude = end_point.altitude;
    }

//    a : x_len = x_altitude : (abs(end_point.altitude - start_point.altitude));
    double x_altitude_diff = (a / x_len) * (abs(end_point.altitude - start_point.altitude));
    intersection_point.altitude = x_altitude_diff + cal_start_altitude;
}

void SpatialHelper::do_get_lines_attr(GDALDataset *dataset,
                                      const string &point_layer_name,
                                      const string &line_layer_name,
                                      const std::vector<long> &line_objectId_list,
                                      std::map<long, Line_Attr> &line_attr_map,
                                      std::map<long, OGRGeometry *> &geometry_map,
                                      bool is_get_point) {

    if (obj_num_list_map_.count((long) dataset) <= 0) {

        string sql = fmt::format("select {},{},{},{},{},{} from {} where {} in({}) ",
                                 field_attr_.fieldName_objectId,
                                 field_attr_.fieldName_line_startObjNum,
                                 field_attr_.fieldName_line_endObjNum,
                                 field_attr_.fieldName_line_diameter,
                                 field_attr_.fieldName_start_altitude,
                                 field_attr_.fieldName_end_altitude,
                                 line_layer_name,
                                 field_attr_.fieldName_objectId,
                                 hzStringUtil::join<long>(line_objectId_list, ','));

        LOG4CPLUS_INFO(MyLogger::getInstance(), "line layer sql: " << sql);

        OGRLayer *line_layer = dataset->ExecuteSQL(sql.c_str(), nullptr, nullptr);
        if (line_layer == nullptr) {
            return;
        }
        if (line_layer->GetFeatureCount() <= 0)
            return;

        // 需要从点层查找的点的, 去重
        set<string> obj_num_list;

        // 遍历所有的线
        OGRFeature *poFeature = line_layer->GetNextFeature();
        while (poFeature) {
            OGRGeometry *pGeometry = poFeature->GetGeometryRef();
            if (!pGeometry->IsValid()) {
                continue;
            }

            long line_objectId = poFeature->GetFieldAsInteger(0);
            const char *start_objNum = poFeature->GetFieldAsString(1);
            const char *end_objNum = poFeature->GetFieldAsString(2);
            const char *line_diameter = poFeature->GetFieldAsString(3);
            double start_altitude = poFeature->GetFieldAsDouble(4);
            double end_altitude = poFeature->GetFieldAsDouble(5);

            Line_Attr line_attr;
            line_attr.object_id = line_objectId;
            line_attr.start_objNum = start_objNum;
            line_attr.end_objNum = end_objNum;
            line_attr.start_altitude = start_altitude;
            line_attr.end_altitude = end_altitude;
            line_attr.pipe_diameter = strtof(line_diameter, nullptr) / 1000;   // 毫米转米
            line_attr_map.emplace(line_objectId, line_attr);

            obj_num_list.insert("'" + string(start_objNum) + "'");
            obj_num_list.insert("'" + string(end_objNum) + "'");

            //
            geometry_map.emplace(line_objectId, pGeometry->clone());

            OGRFeature::DestroyFeature(poFeature);  // 释放

            poFeature = line_layer->GetNextFeature();
        }

        obj_num_list_map_.emplace((long) dataset, obj_num_list);
    }

    // --------------------------------
    // 不需要获取点信息
    if (!is_get_point) {
        return;
    }
    // --------------------------------
    if (point_altitude_map_map_.count((long) dataset) <= 0) {
        auto obj_num_list = obj_num_list_map_[(long) dataset];

        // 查找点层的信息
        string sql = fmt::format("select {} from {} where {} in({})",
                                 field_attr_.fieldName_point_objNum,
                                 point_layer_name,
                                 field_attr_.fieldName_point_objNum,
                                 hzStringUtil::join(obj_num_list, ','));
        LOG4CPLUS_INFO(MyLogger::getInstance(), "point layer sql: " << sql);

        OGRLayer *point_layer = dataset->ExecuteSQL(sql.c_str(), nullptr, nullptr);
        if (point_layer == nullptr) {
            return;
        }
        if (point_layer->GetFeatureCount() <= 0)
            return;

        map<string, Point_Attr> point_altitude_map;
        // 遍历所有的点
        OGRFeature *poFeature = point_layer->GetNextFeature();
        while (poFeature) {
            Point_Attr point_attr{};
            const char *obj_num = poFeature->GetFieldAsString(0);

            OGRGeometry *pGeometry = poFeature->GetGeometryRef();
            if (!pGeometry->IsValid()) {
                continue;
            }
            OGRPoint *pPoint = pGeometry->toPoint();
            point_attr.x = pPoint->getX();
            point_attr.y = pPoint->getY();

            point_altitude_map.emplace(obj_num, point_attr);

            OGRFeature::DestroyFeature(poFeature);  // 释放
            poFeature = point_layer->GetNextFeature();
        }

        // 遍历 line_attr_list, 从 point_altitude_map 中查找 ，设置 x y
        for (auto &line_attr_pair: line_attr_map) {
            Point_Attr &start_point_attr = point_altitude_map.find(line_attr_pair.second.start_objNum)->second;
            line_attr_pair.second.start_point.x = start_point_attr.x;
            line_attr_pair.second.start_point.y = start_point_attr.y;

            Point_Attr &end_point_attr = point_altitude_map.find(line_attr_pair.second.end_objNum)->second;
            line_attr_pair.second.end_point.x = end_point_attr.x;
            line_attr_pair.second.end_point.y = end_point_attr.y;
        }

        point_altitude_map_map_.emplace((long) dataset, point_altitude_map);
    }
}

string SpatialHelper::getResult() const {
    return result_.dump();
}

Field_Attr::Field_Attr(std::string fieldName_objectId,
                       std::string fieldName_point_objNum,
                       std::string fieldName_start_altitude,
                       std::string fieldName_end_altitude,
                       std::string fieldName_line_startObjNum,
                       std::string fieldName_line_endObjNum,
                       std::string fieldName_line_diameter) : fieldName_objectId(std::move(fieldName_objectId)),
                                                              fieldName_point_objNum(std::move(fieldName_point_objNum)),
                                                              fieldName_start_altitude(
                                                                      std::move(fieldName_start_altitude)),
                                                              fieldName_end_altitude(std::move(fieldName_end_altitude)),
                                                              fieldName_line_startObjNum(
                                                                      std::move(fieldName_line_startObjNum)),
                                                              fieldName_line_endObjNum(
                                                                      std::move(fieldName_line_endObjNum)),
                                                              fieldName_line_diameter(
                                                                      std::move(fieldName_line_diameter)) {}

Data_Attr::Data_Attr(string dataPath,
                     string pointLayerName,
                     string lineLayerName,
                     string dataPathOther,
                     string pointLayerNameOther,
                     string lineLayerNameOther) : data_path(std::move(dataPath)),
                                                  point_layer_name(std::move(pointLayerName)),
                                                  line_layer_name(std::move(lineLayerName)),
                                                  data_path_other(std::move(dataPathOther)),
                                                  point_layer_name_other(std::move(pointLayerNameOther)),
                                                  line_layer_name_other(std::move(lineLayerNameOther)) {}
