import os

dir_path = os.path.dirname(os.path.realpath(__file__))
os.chdir(dir_path)

SRC_DIR = r"../shaders"
OUT1= r"../generated/shadersources.cpp"

out = '#include <string>\n'

if not os.path.exists("../generated/"):
    os.mkdir("../generated/")

for filename in os.listdir(SRC_DIR):
    base = os.path.basename(filename)
    stem = os.path.splitext(base)[0]
    
    with open(SRC_DIR + '/' + filename, 'r') as f:
        s = f.read()
        out += r'std::string ShaderSource_' + stem + '=R"SS(' + s + ')SS";\n'
        
f = open(OUT1, 'w')
f.write(out)
f.close()
