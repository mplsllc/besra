import os
import re

with open('clean_symbols.txt', 'r') as f:
    symbols = set(line.strip() for line in f if line.strip() and not line.startswith('/'))

header_dir = 'netsurf/include/netsurf'
headers = [os.path.join(header_dir, f) for f in os.listdir(header_dir) if f.endswith('.h')]

stubs = []
found_symbols = set()

# A very naive regex to match C function declarations
decl_pattern = re.compile(r'((?:[a-zA-Z_][a-zA-Z0-9_]*\s+\*?)+)\s*(gui_[a-zA-Z0-9_]+)\s*\(([^)]*)\)\s*;')

for header in headers:
    with open(header, 'r') as f:
        content = f.read()
    
    # Clean up comments to avoid matching commented out functions
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    content = re.sub(r'//.*', '', content)
    
    for match in decl_pattern.finditer(content):
        ret_type = match.group(1).strip()
        name = match.group(2).strip()
        args = match.group(3).strip()
        
        if name in symbols:
            found_symbols.add(name)
            ret_val = ''
            if ret_type == 'void':
                ret_val = ''
            elif '*' in ret_type or 'pointer' in ret_type.lower():
                ret_val = 'return NULL;'
            elif 'nserror' in ret_type:
                ret_val = 'return NSERROR_OK;'
            elif 'bool' in ret_type:
                ret_val = 'return false;'
            else:
                ret_val = 'return 0;'
                
            stub = f"extern \"C\" {ret_type} {name}({args}) {{\n    {ret_val}\n}}\n"
            stubs.append(stub)

with open('netsurf/frontends/qt6/stubs.cpp', 'w') as f:
    f.write("#include <stdlib.h>\n#include <stdbool.h>\n")
    f.write("#include \"utils/errors.h\"\n")
    f.write("#include \"netsurf/bitmap.h\"\n")
    f.write("#include \"netsurf/clipboard.h\"\n")
    f.write("#include \"netsurf/corewindow.h\"\n")
    f.write("#include \"netsurf/download.h\"\n")
    f.write("#include \"netsurf/fetch.h\"\n")
    f.write("#include \"netsurf/layout.h\"\n")
    f.write("#include \"netsurf/misc.h\"\n")
    f.write("#include \"netsurf/search.h\"\n")
    f.write("#include \"netsurf/window.h\"\n\n")
    f.write("\n".join(stubs))

missing = symbols - found_symbols
if missing:
    print(f"Failed to find declarations for {len(missing)} symbols:")
    for sym in missing:
        print(f"  {sym}")
else:
    print(f"Successfully generated {len(symbols)} stubs.")
