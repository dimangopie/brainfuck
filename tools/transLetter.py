#!/usr/bin/env python
import argparse

output_file: str = "output.bf"
input_file:str = "input.bf"

import io

# 与 C 中常量保持一致，可自行修改
BRAINFUCK_TOKEN_PLUS        = 'o' # '+'  ,  'O'  ,  'o'
BRAINFUCK_TOKEN_MINUS       = 'i' # '-'  ,  'I'  ,  'i'
BRAINFUCK_TOKEN_PREVIOUS    = 'a' # '<'  ,  'A'  ,  'a'
BRAINFUCK_TOKEN_NEXT        = 'e' # '>'  ,  'E'  ,  'e'
BRAINFUCK_TOKEN_OUTPUT      = '.' # '.'  ,  '.'  ,  '.'
BRAINFUCK_TOKEN_INPUT       = ',' # ','  ,  ','  ,  ','
BRAINFUCK_TOKEN_LOOP_START  = 'u' # '['  ,  'U'  ,  'u'
BRAINFUCK_TOKEN_LOOP_END    = 'y' # ']'  ,  'Y'  ,  'y'
BRAINFUCK_TOKEN_BREAK       = '#' # '#'  ,  '!'  ,  '#'

# 字符映射表：每组第 0 个元素是目标字符
OTHER_CHAR_TABLE = [
    [BRAINFUCK_TOKEN_PLUS               , '+', 'o', 'O'],
    [BRAINFUCK_TOKEN_MINUS              , '-', 'i', 'I'],
    [BRAINFUCK_TOKEN_PREVIOUS           , '<', 'a', 'A'],
    [BRAINFUCK_TOKEN_NEXT               , '>', 'e', 'E'],
    [BRAINFUCK_TOKEN_OUTPUT             , '.', '.', '.',],
    [BRAINFUCK_TOKEN_INPUT              , ',', ',', ',',],
    [BRAINFUCK_TOKEN_LOOP_START         , '[', 'u', 'U'],
    [BRAINFUCK_TOKEN_LOOP_END           , ']', 'y', 'Y'],
    [BRAINFUCK_TOKEN_BREAK		        , '#', '?', '!'],
]

# 扁平化为 dict：{ 源字符 : 目标字符 }
_TRANSLATE = {}
for row in OTHER_CHAR_TABLE:
    target, *sources = row
    for s in sources:
        _TRANSLATE[s] = target


def head_preprocessing_line(_line: str) -> (str, bool):
    if _line.startswith("#!"):
        return _line, True
    return _line, False

def preprocessing_line(_line: str) -> str:
    """
    逐个字符读取并做预处理：
    2. 跳过 // 注释行。
    3. 跳过首行的 #! shebang。
    4. 将 o/O → +, i/I → - 等替换。
    返回处理后的单个字符；读到文件末尾返回 ''
    """
    tmp_strings = _line.split("//", 1)
    remain_string = tmp_strings[0]
    tmp_line = ""
    for _char in remain_string:
        tmp_char = _TRANSLATE.get(_char)
        if tmp_char:
            tmp_line += tmp_char
        else:
            tmp_line += _char
    if len(tmp_strings) == 1:
        return tmp_line
    else:
        annotation_string = tmp_strings[1]
        return tmp_line + "//" + annotation_string

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="letters 转换 letters")
    parser.add_argument("-i", required=False, help="输入文件的路径")
    parser.add_argument("-o", required=False, help="输出文件的路径")
    args = parser.parse_args()
    if args.i is not None:
        input_file = args.i
    if args.o is not None:
        output_file = args.o

    lines: list[str] = []
    with io.open(input_file, "r", encoding="utf-8") as f:
        for l in f:
            lines.append(l)

    index: int = 0
    for i in range(len(lines)):
        null, isHead = head_preprocessing_line(l)
        if not isHead:
            index = i + 1
            break
    for i in range(index, len(lines)):
        lines[i] = preprocessing_line(lines[i])

    print("".join(lines))

    with io.open(output_file, "w", encoding="utf-8") as f:
        for l in lines:
            f.write(l)
