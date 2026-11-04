"""Mechanically split ASMifier member blocks below javac's method-size limit.

This changes only the offline assembler, not the runtime instructions it emits.
"""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]/'src/information/instruction-source'
LITERALS = re.compile(r'"(?:[^"\\]|\\.)*"|\'(?:[^\'\\]|\\.)*\'')
DECLARATIONS = ('FieldVisitor fieldVisitor;\nRecordComponentVisitor recordComponentVisitor;\n'
                'MethodVisitor methodVisitor;\nAnnotationVisitor annotationVisitor0;\n')

for source in ROOT.rglob('*.java'):
    text = source.read_text(encoding='utf-8')
    if 'private static void member_' in text:
        continue
    lines = text.splitlines(keepends=True)
    start = next(i for i, line in enumerate(lines) if line.startswith('public static byte[] dump'))
    result = lines[:start+1]
    members = []
    i=start+1
    while i<len(lines):
        if lines[i].strip()!='{' or lines[i]!='{\n':
            result.append(lines[i]);i+=1;continue
        block=[];depth=0
        while i<len(lines):
            line=lines[i];block.append(line);i+=1
            code=LITERALS.sub('',line)
            depth+=code.count('{')-code.count('}')
            if depth==0:break
        if depth!=0:raise RuntimeError('Unbalanced member block: '+str(source))
        name='member_'+str(len(members))
        result.append(name+'(classWriter);\n')
        members.append('private static void '+name+'(ClassWriter classWriter) throws Exception {\n'
                       +DECLARATIONS+''.join(block[1:-1])+'}\n')
    # The source ends with the dump-method brace followed by the class brace.
    last=next(i for i in range(len(result)-1,-1,-1) if result[i].strip()=='}')
    result[last:last]=members
    source.write_text(''.join(result),encoding='utf-8',newline='\n')
print('INFORMATION_ASSEMBLER_SPLIT_PASS')
