def load_obj(filename, out_file):
    out = open(out_file, 'w')
    with open(filename, 'r') as f:
        lines = f.readlines()
        part = 0
        shape_name = ''
        for line in lines:
            tokens = [x.strip() for x in line.split(' ') if x]
            if tokens[0] == 'g':
                continue
            if tokens[0] == 'o':
                shape_name = '.'.join(tokens[1:])
                part = 0
                continue
            if tokens[0] == 'usemtl':
                path = tokens[1].replace('\\', '/')
                out.write('o {0}.{1}.{2}\n'.format(shape_name, path, part))
                out.write(line)
                part += 1
            else:
                out.write(line)


import sys

if __name__ == '__main__':
    load_obj(sys.argv[1], sys.argv[2])
