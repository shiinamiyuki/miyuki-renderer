//
// Created by Shiina Miyuki on 2019/4/8.
//

#include <io/image.h>
#include <denoiser/nlm.h>
#include <denoiser/bilateral.hpp>

using namespace Miyuki;
using namespace Miyuki::Denoising;

int main() {
    try {
        IO::Image in("fireplace_room.png");
        IO::Image out(in.width, in.height);
//        BoxFilterNaive(in, out, 7);
        NLMeans(in, out, 21, 7, 0.01, 0.05);
        out.save("data/debug/nlm.png");
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}
