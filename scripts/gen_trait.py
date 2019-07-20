def gen_trait(N):
    s = 'template<'
    for i in range(1, 1 + N):
        s += 'class T' + str(i)
        if i != N:
            s += ', '
    s += '>\n'
    s += '''struct Trait<'''
    for i in range(1, 1 + N):
        s += 'T' + str(i)
        if i != N:
            s += ', '
    s += '>{\n'
    s += '''    template<class T>
    T* get()const {
        static_assert(false, "Does not have that trait");
    }
'''
    for i in range(1, 1 + N):
        tmp =  '''  template<>
    ## * get<##>()const {
        return #ptr;
    }
'''
        tmp = tmp.replace('##', 'T' + str(i))
        tmp = tmp.replace('#ptr', 'ptr'+str(i))
        s += tmp
    s += '''
    template<class Any>
    Trait(Any p){
        reset(p);
    }
    Trait(){}
    template<class Any>
    Trait& operator = (Any p){
        reset(p);
        return *this;
    }
    '''
    s += '''template<class Any>
    void reset(Any* p) {
'''
    for i in range(1, 1 + N):
        s += '\t\tptr' + str(i) + '= static_cast<T'+str(i)+'*>(p);\n'
    s += '\t}\n'
    s += '''template<class... T>
    void reset(const Trait<T...> & trait){
'''
    for i in range(1, 1 + N):
        s += '\t\tptr' + str(i) + '= trait.get<T'+str(i)+'>();\n'
    s += '\t};\n'
    s += '\tprivate:\n'
    for i in range(1, 1 + N):
        s += '\tT'+str(i) + '* ptr' + str(i) + ' = nullptr;\n'
    s += '\t};\n'
    return s


f = open('../include/miyuki/trait.hpp', 'w')
f.write('''#ifndef MIYUKI_TRAIT_HPP
#define MIYUKI_TRAIT_HPP
// This file is auto-generated
// Do not edit!!
#include <memory.h>
namespace Miyuki{
template<class...>
struct Trait;
''')
for i in range(1, 17):
    f.write(gen_trait(i))
f.write('''

using unique_void_ptr = std::unique_ptr<void, void(*)(void*)>;

template<class T>
unique_void_ptr make_unique_void(T * ptr){
    return unique_void_ptr(ptr, [](void  * p){
        auto true_p = static_cast<T*>(p);
        delete true_p;
    });
};
unique_void_ptr make_unique_void_nullptr(){
    return unique_void_ptr(nullptr, [](void  * p){
    
    });
}

template<class... T>
struct BoxedTrait;
template<class... T>
struct BoxedTrait{
    template<class... U>
    friend struct BoxedTrait;
    BoxedTrait():ptr(make_unique_void_nullptr()){}
    template<class Any>
    BoxedTrait(Any * p):ptr(std::move(make_unique_void(p))){
        trait.reset(p);
    }
    template<class... U>
    BoxedTrait(BoxedTrait<U...> && box):ptr(std::move(box.ptr)){
        trait = box.trait;
    }
    template<class Any>
    void reset(Any * p){
        ptr = make_unique_void(p);
        trait.reset(p);
    }
    const Trait<T...> * operator ->()const{
        return &trait;
    }
    const Trait<T...> & operator*()const{
        return trait;
    }
    void * release(){
        trait.reset(nullptr);
        return ptr.relesase();
    }
private:
    Trait<T...> trait;
    unique_void_ptr ptr; 
};

template<class... T, class... U>
BoxedTrait&& static_boxed_trait_upcast(BoxedTrait<U...>&& from){
    auto 
}

''')
f.write('}\n')

f.write("#endif")