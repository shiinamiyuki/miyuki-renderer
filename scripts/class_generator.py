def camel_to_snake(camel:str, upper:bool)->str:
    lst = []
    buffer = ''
    for i in camel:
        if i.isupper():
            if buffer:
                lst.append(buffer)
            buffer = i
        else:
            buffer += i
    if buffer:
        lst.append(buffer)
    result = '_'.join([x.upper() if upper else x.lower() for x in lst])
    return result

class Globals:
    def __init__(self):
        self.clear()
        
    def clear(self):
        self.supers = dict()
        self.classes = dict()

    def gen_super_meta(self, _super):
        s = ' ' * 4 + _super + 'Type type_tag;\n'
        s += ' ' * 4 + 'union{\n'
        for i in self.supers[_super]:
            s += ' ' * 8 + i.name + ' ' + camel_to_snake(i.name, False) + ';\n'
        s += ' ' * 4 + '};\n'
        s += ' ' * 4 + 'META_' + camel_to_snake(_super, True) + '()\n'
        return s

    def gen_dispatcher(self, _super):
        macro = 'DISPATCH_' + camel_to_snake(_super, True)
        s = '#define ' + macro + '(method,object, ...) \\\n'
        # s += 'ret ' + camel_to_snake(_super, False) + '##_##method(' + _super + '* pObject,' + '__VA_ARGS__){\\\n'
        s += ' ' * 4 + 'switch(object->type_tag) {\\\n'
        for i in self.supers[_super]:
            s += ' ' * 4 + 'case {0}:\\\n'.format(i.type_tag)
            s += ' ' * 8 + 'return ' + camel_to_snake(i.name, False) + '##_##method(({0} *)&object->{1}, __VA_ARGS__);\\\n'.format(i.name, camel_to_snake(i.name, False))
        s += ' ' * 4 + '};\\\n'
        s += ' ' * 4 + 'assert(0);\n\n'
        return s


    def gen_super(self, _super):
        c = self.classes[_super]
        attrs = ''
        for i in c.attributes:
            attrs += ' '* 4 + i.to_str() + ';\n'
        s = \
        """
typedef struct #NAME{
#META
#ATTR
}#NAME;

""".replace("#NAME", _super).replace('#ATTR', attrs).replace('#META', self.gen_super_meta(_super))
        s += self.gen_dispatcher(_super)
        return s
    
    def gen_type_tag(self, _super):
        s = 'enum ' + _super + 'Type{\n'
        s += ' ' *4  + _super.upper() + '_NONE,\n'
        for i in self.supers[_super]:
            s += ' ' * 4 + i.type_tag + ',\n'
        s += '};\n'
        return s

    def gen_type_tags(self):
        s = ''
        for _super in self.supers:
            s += self.gen_type_tag(_super)
        return s

    def gen(self):
        s = ''
        s += self.gen_type_tags()
        for i in self.classes:
            s += 'struct ' + self.classes[i].name + ';\n'        
        for i in self.classes:
            if self.classes[i].super:
                s += self.classes[i].gen_def()
            if self.classes[i].is_super():
                s += self.classes[i].gen_meta()
        for i in self.classes:
            if not self.classes[i].super:
                s += self.gen_super(i)
                s += self.classes[i].gen_ctor()
            else:
                s += self.classes[i].gen_ctor()
        return s
    
globals = Globals()

class KernelAttribute:
    def __init__(self, name:str, type:str):
        self.is_pointer = type.strip().endswith('*')
        self.name = name
        self.type = type
    
    def to_str(self):
        return self.type + ' ' + self.name
    
class KernelClass:
    def __init__(self, name:str, _super:str):
        self.attributes = []
        self.name = name
        self.type_tag = camel_to_snake(name, True)
        self.super = _super
        self.comment = ''
        global globals
        if self.super:
            if self.super not in globals.supers:
                globals.supers[self.super] = [self]
            else:
                globals.supers[self.super].append(self)
        globals.classes[self.name] = self

    def get_ctor_name(self):
        return 'create_' + camel_to_snake(self.name, False)

    def add_attr(self, attr):
        self.attributes.append(attr)
        return self

    def gen_ctor(self):
        arg_type = self.name 
        if self.super:
            arg_type = self.super
        s = 'MYK_KERNEL_FUNC_INLINE void ' + self.get_ctor_name() + '(' + arg_type + '* object){\n'
        o = 'object->'
        if self.super:
            o = 'object->' + camel_to_snake(self.name, False) + '.'
        for a in self.attributes:
            if a.is_pointer:
                s += ' ' * 4 + o + a.name + ' = NULL;\n'
        if self.super:
            s += ' ' * 4 + 'object->type_tag = ' + self.type_tag + ';\n'
        s += '}\n'
        return s
    def is_super(self):
        return not self.super

    def gen_meta(self):
        assert self.is_super()
        s = '#define META_' + camel_to_snake(self.name, True) + '() '
        s += "{0}()=default;{0}(const {0}& rhs)".format(self.name) + '{'
        global globals
        s += 'switch(type_tag){'
        for i in globals.supers[self.name]:
            d = i
            s += 'case {0}:'.format(camel_to_snake(d.name, True)) + '  '
            s += '{0} = rhs.{0};'.format(camel_to_snake(d.name, 0))
        s += '} }\n'
        return s

    def gen_comment(self):
        return '/*' + self.comment + '*/' if self.comment else ''

    def gen_def(self)->str:
        s = self.gen_comment() + \
            """
typedef struct #NAME{
#ATTR
}#NAME;

""".replace("#NAME", self.name)
        attrs = ''
        for i in self.attributes:
            attrs += ' '* 4 + i.to_str() + ';\n'

        s = s.replace("#ATTR", attrs)
        return s


def create_class(name, data:dict):
    _super = data['super']
    _class = KernelClass(name,_super)
    if 'attr' in data:
        attrs = data['attr']
        for attr in attrs:
            ty = attrs[attr]
            _class.add_attr(KernelAttribute(attr, ty))  
    if 'comment' in data:
        _class.comment = data['comment']
    return _class
def create_classes(data):
    for i in data:
        create_class(i, data[i])
