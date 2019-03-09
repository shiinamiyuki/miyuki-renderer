//
// Created by Shiina Miyuki on 2019/3/7.
//

#include "progress.h"

namespace Miyuki {

    void ProgressReporter::update() {
        counter++;
        int cnt = counter;
        callback(cnt, total);
    }

    Float ProgressReporter::estimatedTimeToFinish() const {
        return elapsedSeconds() * (Float(total) / counter - 1);
    }
}