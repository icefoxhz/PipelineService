#ifndef PIPELINESERVICE_SPATIALHELPER_H
#define PIPELINESERVICE_SPATIALHELPER_H

#include <vector>
#include <set>
#include <map>
#include <gdal_priv.h>
#include "../json.hpp"


struct Point_Attr {
    double x;               // x 坐标   不从属性表里读取
    double y;               // y 坐标   不从属性表里读取
    double altitude;        // 管点高程
};

struct Line_Attr {
    long object_id;              // 线对象id
    double pipe_diameter;        // 管径
    Point_Attr start_point;      // 起点
    Point_Attr end_point;        // 终点
    double start_altitude;       // 起点高程
    double end_altitude;         // 终点高程
    std::string start_objNum;    // 起点物探点号
    std::string end_objNum;      // 终点物探点号
};

struct Field_Attr {
    std::string fieldName_objectId;           // 线层 objectId字段名. 只要线层的就行，点层的不需要
    std::string fieldName_point_objNum;            // 点层 物探点号字段名
    std::string fieldName_start_altitude;          // 点层 管顶起点高程字段名
    std::string fieldName_end_altitude;          // 点层 管顶终点高程字段名
    std::string fieldName_line_startObjNum;        // 线层 起点物探点号字段名
    std::string fieldName_line_endObjNum;          // 线层 终点物探点号字段名
    std::string fieldName_line_diameter;          //  线层 管径 字段名

    Field_Attr(std::string fieldName_objectId,
               std::string fieldName_point_objNum,
               std::string fieldName_start_altitude,
               std::string fieldName_end_altitude,
               std::string fieldName_line_startObjNum,
               std::string fieldName_line_endObjNum,
               std::string fieldName_line_diameter);
};

struct Data_Attr {
    std::string data_path;                  // 数据文件路径
    std::string point_layer_name;           // 点层名称
    std::string line_layer_name;            // 线层名称

    std::string data_path_other;             // 数据文件路径
    std::string point_layer_name_other;      // 点层名称
    std::string line_layer_name_other;       // 线层名称

    Data_Attr(std::string dataPath,
              std::string pointLayerName,
              std::string lineLayerName,
              std::string dataPathOther,
              std::string pointLayerNameOther,
              std::string lineLayerNameOther);
};

enum do_distance_type{
    horizontal = 1,   // 水平距离
    vertical = 2,     // 垂直距离
    both = 3          // 水平距离 和 垂直距离
};

class SpatialHelper {
public:
    SpatialHelper(Field_Attr &field_attr, Data_Attr &data_attr);
    ~SpatialHelper();

    void run(do_distance_type& dis_type);

    /**
     * 设置要计算的一个图层的id列表
     * @param objectIdList
     */
    void setObjectIdList(const std::vector<long> &objectIdList);

    /**
     * 设置要计算的另一个图层的id列表
     * @param objectIdOtherList
     */
    void setObjectIdOtherList(const std::vector<long> &objectIdOtherList);

    std::string getResult() const;

private:
    GDALDataset *dataset_ = nullptr;            // 一个文件的dataset
    GDALDataset *dataset_other_ = nullptr;      // 另一个文件的dataset

    Field_Attr field_attr_;     //  字段相关的属性
    Data_Attr data_attr_;       // 数据

    std::vector<long> object_id_list_;           //  一个线层的 objectid 列表
    std::vector<long> object_id_other_list_;     //  另一个线层的 objectid 列表

    std::map<long, std::set<std::string>> obj_num_list_map_;
    std::map<long, std::map<std::string, Point_Attr>> point_altitude_map_map_;
/*    std::set<std::string> obj_num_list_;
    std::map<std::string, Point_Attr> point_altitude_map_;*/

    const int BATCH_SIZE = 1000;    // 批量处理的大小

    nlohmann::json result_;      // 计算结果

private:
    bool init_succeed();

    /**
     * 获取线与线交点
     * @param geom
     * @param geom_other
     * @param point
     */
    static void get_lines_intersection_point(class OGRGeometry *geom, class OGRGeometry *geom_other, class OGRPoint **point);

    /**
     * geo转line
     * @param geom
     * @param line
     */
    static void get_line_from_geometry(class OGRGeometry *geom, class OGRLineString **line);

    /**
     * 计算交点的高程
     * @return
     */
    static void cal_point_on_line_altitude(const Point_Attr &start_point,
                                           const Point_Attr &end_point,
                                           Point_Attr &intersection_point);
    /**
     * 水平间距(2个之间)
     * @param geom
     * @param geom_other
     * @return
     */
    static double cal_horizontal_spacing_between_two(class OGRGeometry *geom, class OGRGeometry *geom_other);

    /**
    * 计算交点的高程
    * @param line_attr
    * @param point
    * @param intersection_point
    */
    static void get_intersection_point_altitude(const Line_Attr &line_attr,
                                                const OGRPoint *point, Point_Attr &intersection_point);

    /**
     * 释放实体
     * @param geometry_map
     */
    static void releaseGeometries(std::map<long, OGRGeometry *> &geometry_map);

    /**
    * 垂直间距(2个之间)
    * @param geom
    * @param line_attr
    * @param geom_other
    * @param line_attr_other
    * @return
    */
    double cal_vertical_spacing_between_two(class OGRGeometry *geom, Line_Attr &line_attr,
                                            class OGRGeometry *geom_other, Line_Attr &line_attr_other);

    /**
     *  执行水平距离计算
     * @param geometry_map
     * @param geometry_map_other
     */
    void do_horizontal_spacing(std::map<long, OGRGeometry *> &geometry_map,
                               std::map<long, OGRGeometry *> &geometry_map_other,
                               std::map<long, Line_Attr>& line_attr_map,
                               std::map<long, Line_Attr>& line_attr_other_map);

    /**
     * 执行垂直距离计算
     * @param geometry_map
     * @param geometry_map_other
     */
    void do_vertical_spacing(std::map<long, OGRGeometry *> &geometry_map,
                             std::map<long, OGRGeometry *> &geometry_map_other,
                             std::map<long, Line_Attr>& line_attr_map,
                             std::map<long, Line_Attr>& line_attr_other_map);

    bool init_lines(std::map<long, OGRGeometry *> &geometry_map,
                    std::map<long, OGRGeometry *> &geometry_other_map,
                    std::map<long, Line_Attr> &line_attr_map,
                    std::map<long, Line_Attr> &line_attr_other_map,
                    bool is_get_point = true);

    /**
     * 获取线数组中每条线的起点和终点的高程
     * @param point_layer_name
     * @param line_layer_name
     * @param line_objectId_list
     * @param line_attr_map
     * @param geometry_map
     * @param is_get_point
     */
    void do_get_lines_attr(GDALDataset *dataset,
                           const std::string &point_layer_name,
                           const std::string &line_layer_name,
                           const std::vector<long> &line_objectId_list,
                           std::map<long, Line_Attr> &line_attr_map,
                           std::map<long, OGRGeometry *> &geometry_map,
                           bool is_get_point = true);

    /**
     *  生成结果
     * @param key_name
     * @param key_val
     * @param distance
     */
    void insertResult(const std::string &key_name, const std::string &key_val, double distance);
};


#endif //PIPELINESERVICE_SPATIALHELPER_H
