#pragma once
#include <QVector>
#include <QVector3D>

class RouteData {
public:
    // 凸包顶点（三维坐标，包含航高）
    QVector<QVector3D> convexHullPoints;

    // 生成的航线路径点
    QVector<QVector3D> flightPath;

    // 凸包生成算法（基于 XZ 平面投影）
    void calculateConvexHull();

    // 根据航高生成平行航线
    void generateFlightPath(float height, float spacing);
};