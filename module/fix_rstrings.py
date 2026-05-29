"""修复所有 C++ 文件中的 R"(...)" 语法问题。
MSVC 的 R"(...)" raw string 中如果有 ) 会提前终止。
改为使用 R"json(...)json" 格式或普通转义字符串。
"""
import re
import glob
import os

def fix_file(fpath):
    with open(fpath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    if 'R"(' not in content:
        return False
    
    lines = content.split('\n')
    new_lines = []
    changed = False
    
    for line in lines:
        if 'R"(' not in line:
            new_lines.append(line)
            continue
        
        # Replace R"({json})" with R"json({json})json" - uses custom delimiter
        # This avoids ) inside the content terminating the raw string
        new_line = line.replace('R"(', 'R"json(').replace(')"', ')json"')
        
        # But wait - if the content has ) inside, R"json(...)json" still has the same problem
        # because )json" needs ) followed by json" - but content may have ) not followed by json"
        # Actually R"json(...)json" is DELIMITED: the opening is R"json( and closing is )json"
        # So ) inside is fine as long as it's not followed by json" !
        # This should work for JSON strings since they don't contain ")json"
        
        if new_line != line:
            changed = True
        new_lines.append(new_line)
    
    if changed:
        with open(fpath, 'w', encoding='utf-8') as f:
            f.write('\n'.join(new_lines))
        print(f'Fixed: {fpath}')
    return changed

# Fix all cpp files
count = 0
for fpath in glob.glob('**/*.cpp', recursive=True):
    if fix_file(fpath):
        count += 1

print(f'\nTotal files fixed: {count}')
