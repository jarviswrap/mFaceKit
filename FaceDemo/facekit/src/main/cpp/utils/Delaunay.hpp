//
// Created by wilbert on 2026/1/24.
//

#ifndef FACEDEMO_DELAUNAY_HPP
#define FACEDEMO_DELAUNAY_HPP

#include <vector>
#include <algorithm>
#include <cmath>

namespace face {

    struct DPoint {
        float x, y;
        int id; // Original index
    };

    struct DTriangle {
        int p1, p2, p3; // Indices in the point list
        bool isBad;
    };

    struct DEdge {
        int p1, p2;
        bool operator==(const DEdge& other) const {
            return (p1 == other.p1 && p2 == other.p2) || (p1 == other.p2 && p2 == other.p1);
        }
    };

    class Delaunay {
    public:
        // Bowyer-Watson algorithm
        static std::vector<int> triangulate(const std::vector<DPoint>& points) {
            std::vector<DTriangle> triangles;
            
            // 1. Determine super-triangle
            float minX = points[0].x;
            float minY = points[0].y;
            float maxX = minX;
            float maxY = minY;

            for (const auto& p : points) {
                if (p.x < minX) minX = p.x;
                if (p.x > maxX) maxX = p.x;
                if (p.y < minY) minY = p.y;
                if (p.y > maxY) maxY = p.y;
            }

            float dx = maxX - minX;
            float dy = maxY - minY;
            float deltaMax = std::max(dx, dy);
            float midX = (minX + maxX) / 2.0f;
            float midY = (minY + maxY) / 2.0f;

            // Super triangle vertices (not in the original point list)
            // We append them to the end of a temporary list if needed, or handle indices carefully.
            // Here we use negative indices or indices > points.size() to denote super triangle vertices?
            // Simpler: treat input 'points' as fixed, and we manage super vertices virtually.
            // Or better: Work with a local copy of points including super triangle.
            
            std::vector<DPoint> allPoints = points;
            
            DPoint st1 = {midX - 20 * deltaMax, midY - deltaMax, -1};
            DPoint st2 = {midX, midY + 20 * deltaMax, -1};
            DPoint st3 = {midX + 20 * deltaMax, midY - deltaMax, -1};
            
            int stIdx1 = allPoints.size(); allPoints.push_back(st1);
            int stIdx2 = allPoints.size(); allPoints.push_back(st2);
            int stIdx3 = allPoints.size(); allPoints.push_back(st3);

            triangles.push_back({stIdx1, stIdx2, stIdx3, false});

            // 2. Add each point
            for (int i = 0; i < points.size(); ++i) {
                std::vector<DTriangle> badTriangles;
                
                // Find bad triangles
                for (auto& t : triangles) {
                    if (isPointInCircumcircle(allPoints[i], allPoints[t.p1], allPoints[t.p2], allPoints[t.p3])) {
                        t.isBad = true;
                        badTriangles.push_back(t);
                    }
                }

                std::vector<DEdge> polygon;
                for (const auto& t : badTriangles) {
                    addEdgeToPolygon(polygon, {t.p1, t.p2});
                    addEdgeToPolygon(polygon, {t.p2, t.p3});
                    addEdgeToPolygon(polygon, {t.p3, t.p1});
                }

                // Remove bad triangles
                triangles.erase(std::remove_if(triangles.begin(), triangles.end(), [](const DTriangle& t){
                    return t.isBad;
                }), triangles.end());

                // Re-triangulate
                for (const auto& edge : polygon) {
                    triangles.push_back({edge.p1, edge.p2, i, false});
                }
            }

            // 3. Clean up super triangle
            std::vector<int> indices;
            for (const auto& t : triangles) {
                if (t.p1 >= points.size() || t.p2 >= points.size() || t.p3 >= points.size()) {
                    continue;
                }
                indices.push_back(t.p1);
                indices.push_back(t.p2);
                indices.push_back(t.p3);
            }

            return indices;
        }

    private:
        static bool isPointInCircumcircle(const DPoint& p, const DPoint& a, const DPoint& b, const DPoint& c) {
            float ax_ = a.x - p.x;
            float ay_ = a.y - p.y;
            float bx_ = b.x - p.x;
            float by_ = b.y - p.y;
            float cx_ = c.x - p.x;
            float cy_ = c.y - p.y;

            float det = (ax_ * ax_ + ay_ * ay_) * (bx_ * cy_ - cx_ * by_) -
                        (bx_ * bx_ + by_ * by_) * (ax_ * cy_ - cx_ * ay_) +
                        (cx_ * cx_ + cy_ * cy_) * (ax_ * by_ - bx_ * ay_);

            // Check winding order to ensure correct sign? 
            // Assuming CCW or CW consistency.
            // Actually, if we just want "inside", typically det > 0 means inside if points are CCW.
            // But let's use a simpler distance check if precision is an issue? 
            // No, determinant is standard. 
            // However, need to be careful with floating point.
            // Let's assume standard CCW triangles.
            
            // For robustness, we can compute circumcenter and radius squared.
            float D = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
            if (std::abs(D) < 1e-6) return false; // Collinear

            float center_x = ((a.x * a.x + a.y * a.y) * (b.y - c.y) + (b.x * b.x + b.y * b.y) * (c.y - a.y) + (c.x * c.x + c.y * c.y) * (a.y - b.y)) / D;
            float center_y = ((a.x * a.x + a.y * a.y) * (c.x - b.x) + (b.x * b.x + b.y * b.y) * (a.x - c.x) + (c.x * c.x + c.y * c.y) * (b.x - a.x)) / D;
            
            float radius2 = (center_x - a.x) * (center_x - a.x) + (center_y - a.y) * (center_y - a.y);
            float dist2 = (center_x - p.x) * (center_x - p.x) + (center_y - p.y) * (center_y - p.y);
            
            return dist2 <= radius2;
        }

        static void addEdgeToPolygon(std::vector<DEdge>& polygon, const DEdge& edge) {
            auto it = std::find(polygon.begin(), polygon.end(), edge);
            if (it != polygon.end()) {
                // If edge exists, it's shared, so remove it (internal edge)
                polygon.erase(it);
            } else {
                polygon.push_back(edge);
            }
        }
    };

} // face

#endif //FACEDEMO_DELAUNAY_HPP
