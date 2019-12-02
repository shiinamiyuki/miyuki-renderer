import os
import sys

LICENSE = r'''// MIT License
// 
// Copyright (c) 2019 椎名深雪
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
'''



def f(SRC_DIR):
    for parent, dirs, files in os.walk(SRC_DIR):
	    for file in files:
		    if file in {"sobolmat.hpp"}:
			    continue
		    out = ''
		    path = os.path.join(parent, file)
		    with open(path, 'r',encoding='utf-8') as f:
			    s = f.read()
			    if not s.startswith('// MIT License'):
				    out = LICENSE + s
				    print('add license to ' + path)
			    else:
				    out = s
		    with open(path, 'w',encoding='utf-8') as f:
			    f.write(out)

f(r'../src')
f(r'../miyuki.foundation/include')
f(r'../miyuki.foundation/src')