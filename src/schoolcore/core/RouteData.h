#pragma once
#include <QVector>
#include <QVector3D>

class RouteData {
public:
    // ͹�����㣨��ά���꣬�������ߣ�
    QVector<QVector3D> convexHullPoints;

    // ���ɵĺ���·����
    QVector<QVector3D> flightPath;

    // ͹�������㷨������ XZ ƽ��ͶӰ��
    void calculateConvexHull();

    // ���ݺ�������ƽ�к���
    void generateFlightPath(float height, float spacing);
};