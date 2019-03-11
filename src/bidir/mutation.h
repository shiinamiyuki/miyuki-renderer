//
// Created by Shiina Miyuki on 2019/3/11.
//

#ifndef MIYUKI_MUTATION_H
#define MIYUKI_MUTATION_H
namespace Miyuki{
    namespace Bidir{
        struct MutationType{
            static const int none = 0;
            static const int bidirMutation = 1;
            static const int lensPerturb = 2;
            static const int multichain = 4;
            int value;
            operator int()const{
                return value;
            }
        };
    }
}
#endif //MIYUKI_MUTATION_H
