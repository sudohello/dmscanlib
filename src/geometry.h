#ifndef __INC_STRUCTS_H
#define __INC_STRUCTS_H

#include <ostream>
#include <limits>
#include <algorithm>
#include <memory>

using namespace std;

template<typename T>
struct Point {
   const T x;
   const T y;

   Point(const T _x, const T _y) :
         x(_x), y(_y) {
   }

   Point(const Point<T> & point) :
         x(point.x), y(point.y)
      {

      }

   unique_ptr<const Point<T> > scale(const T factor) const {
      return unique_ptr<const Point<T> >(new Point<T>(x * factor, y * factor));
   }

   unique_ptr<const Point<T> > translate(const Point<T> distance) const {
      return unique_ptr<const Point<T> >(new Point<T>(x + distance.x, y + distance.y));
   }
};

template<typename T>
struct Rect;

template<typename T>
struct BoundingBox {
   BoundingBox(const T x1, const T y1, const T x2, const T y2) :
         points( { Point<T>(x1, y1), Point<T>(x2, y2) }) {

   }

   BoundingBox(const BoundingBox<T> & r) :
         points( {
               Point<T>(r.points[0].x, r.points[0].y),
                  Point<T>(r.points[1].x, r.points[1].y)
                  }) {

   }

   const Point<T> points[2];
};

template<typename T>
ostream & operator<<(ostream &os, const BoundingBox<T> & m) {
   os << "(" << m.points[0].x << ", " << m.points[0].y << "), " << "("
      << m.points[1].x << ", " << m.points[1].y << ")";
   return os;
}

template<typename T>
ostream & operator<<(ostream &os, const Rect<T> & m) {
   os << "(" << m.corners[0].x << ", " << m.corners[0].y << "), " << "("
      << m.corners[1].x << ", " << m.corners[1].y << "), " << "("
      << m.corners[2].x << ", " << m.corners[2].y << "), " << "("
      << m.corners[3].x << ", " << m.corners[3].y << ")";
   return os;
}

template<typename T>
struct Rect {
   Rect(const Rect<T> &r) :
         corners({ r.corners[0], r.corners[1], r.corners[2], r.corners[3] })
      {

      }
   Rect(const Point<T> & pt1, const Point<T> & pt2, const Point<T> & pt3,
        const Point<T> & pt4) :
         corners({ pt1, pt2, pt3, pt4 })
      {

      }

   Rect(const T x1, const T y1, const T x2, const T y2, const T x3, const T y3, const T x4, const T y4) :
         corners( { Point<T>(x1, y1), Point<T>(x2, y2), Point<T>(x3, y3),
                  Point<T>(x4, y4) }) {

   }

   Rect(const BoundingBox<T> & bbox) :
         corners({
               Point<T>(bbox.points[0].x, bbox.points[0].y),
                  Point<T>(bbox.points[0].x, bbox.points[1].y),
                  Point<T>(bbox.points[1].x, bbox.points[1].y),
                  Point<T>(bbox.points[1].x, bbox.points[0].y) })
      {

      }

   unique_ptr<const Rect<T> > scale(const T factor) const {
      unique_ptr<const Point<T> > pt1 = std::move(corners[0].scale(factor));
      unique_ptr<const Point<T> > pt2 = std::move(corners[1].scale(factor));
      unique_ptr<const Point<T> > pt3 = std::move(corners[2].scale(factor));
      unique_ptr<const Point<T> > pt4 = std::move(corners[3].scale(factor));
      return unique_ptr<const Rect<T> >(new Rect<T>(*pt1, *pt2, *pt3, *pt4));
   }

   unique_ptr<const Rect<T> > translate(const Point<T> & distance) const {
      unique_ptr<const Point<T> > pt1 = std::move(corners[0].translate(distance));
      unique_ptr<const Point<T> > pt2 = std::move(corners[1].translate(distance));
      unique_ptr<const Point<T> > pt3 = std::move(corners[2].translate(distance));
      unique_ptr<const Point<T> > pt4 = std::move(corners[3].translate(distance));
      return unique_ptr<const Rect<T> >(new Rect<T>(*pt1, *pt2, *pt3, *pt4));
   }


   unique_ptr<const BoundingBox<T> > getBoundingBox() const {
      T maxX = numeric_limits<T>::min();
      T maxY = numeric_limits<T>::min();

      T minX = numeric_limits<T>::max();
      T minY = numeric_limits<T>::max();

      for (unsigned i = 0; i < 4; ++i) {
         minX = min(minX, corners[i].x);
         minY = min(minY, corners[i].y);

         maxX = max(maxX, corners[i].x);
         maxY = max(maxY, corners[i].y);
      }
      return unique_ptr<const BoundingBox<T> >(
         new BoundingBox<T>(minX, minY, maxX, maxY));
   }

   const Point<T> corners[4];
   friend ostream & operator<<<>(std::ostream & os, const Rect<T> & m);
};

#endif /* __INC_STRUCTS_H */

