//
// Created by Shiina Miyuki on 2019/3/3.
//

#include <math/sampling.h>

namespace Miyuki {
    Point2f ConcentricSampleDisk(const Point2f &u) {
        Point2f uOffset = Point2f{2.0f, 2.0f} * u - Point2f(1, 1);

        if (uOffset.x() == 0 && uOffset.y() == 0)
            return Point2f(0, 0);

        Float theta, r;
        if (std::abs(uOffset.x()) > std::abs(uOffset.y())) {
            r = uOffset.x();
            theta = PI / 4.0f * (uOffset.y() / uOffset.x());
        } else {
            r = uOffset.y();
            theta = PI / 2.0f - PI / 4.0f * (uOffset.x() / uOffset.y());
        }
        return Point2f{r, r} * Point2f(std::cos(theta), std::sin(theta));
    }


}