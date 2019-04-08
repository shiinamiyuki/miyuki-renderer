//
// Created by Shiina Miyuki on 2019/4/8.
//

#include <io/image.h>
#include <denoiser/nlm.h>

using namespace Miyuki;

int main() {
    try {
        IO::Image in("fireplace_room.png"), out;
        NLMeans(in, out, 21, 5, 0.01);
        fmt::print("Done\n");
        out.save("out.png");
        fmt::print("Saved\n");
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}
