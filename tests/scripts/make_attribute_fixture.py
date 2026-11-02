#!/usr/bin/env python3
"""Generate a structurally valid Java 8 class carrying decoder test attributes."""
import struct
import sys

u1=lambda x: struct.pack('>B',x)
u2=lambda x: struct.pack('>H',x)
u4=lambda x: struct.pack('>I',x)
cp=[None]
def utf(s):
    b=s.encode('ascii'); cp.append(u1(1)+u2(len(b))+b); return len(cp)-1
def cls(name):
    ni=utf(name); cp.append(u1(7)+u2(ni)); return len(cp)-1
def integer(v):
    cp.append(u1(3)+u4(v)); return len(cp)-1
def nat(n,d):
    a,b=utf(n),utf(d); cp.append(u1(12)+u2(a)+u2(b)); return len(cp)-1

this=cls('AttributeFixture'); parent=cls('java/lang/Object'); exc=cls('java/lang/Exception')
test=utf('test'); desc=utf('(I)V'); arg=utf('arg'); int_desc=utf('I')
ann_type=utf('LFixture;'); ann_name=utf('value'); value=integer(37); source=utf('AttributeFixture.java')
sig=utf('Ljava/lang/Object;'); nested=utf('Nested'); method_nat=nat('test','(I)V')
names={n:utf(n) for n in [
    'Code','ConstantValue','Exceptions','InnerClasses','EnclosingMethod','Synthetic','Signature','SourceFile',
    'LineNumberTable','LocalVariableTable','LocalVariableTypeTable','Deprecated','RuntimeVisibleAnnotations',
    'RuntimeInvisibleAnnotations','RuntimeVisibleParameterAnnotations','RuntimeInvisibleParameterAnnotations',
    'RuntimeVisibleTypeAnnotations','RuntimeInvisibleTypeAnnotations','AnnotationDefault','BootstrapMethods',
    'MethodParameters','StackMapTable','SourceDebugExtension']}

def attr(name, data): return u2(names[name])+u4(len(data))+data
def annotation(): return u2(ann_type)+u2(1)+u2(ann_name)+u1(ord('I'))+u2(value)
def empty_annotation(): return u2(ann_type)+u2(0)
def table(name, entries): return attr(name,u2(len(entries))+b''.join(entries))

code=b'\xb1'
nested_attrs=[
    attr('StackMapTable',u2(0)),
    table('LineNumberTable',[u2(0)+u2(12)]),
    table('LocalVariableTable',[u2(0)+u2(1)+u2(arg)+u2(int_desc)+u2(0)]),
    table('LocalVariableTypeTable',[u2(0)+u2(1)+u2(arg)+u2(int_desc)+u2(0)]),
]
code_info=u2(0)+u2(1)+u4(len(code))+code+u2(0)+u2(len(nested_attrs))+b''.join(nested_attrs)
method_attrs=[
    attr('Code',code_info),
    attr('Exceptions',u2(1)+u2(exc)),
    attr('MethodParameters',u1(1)+u2(arg)+u2(0)),
    attr('RuntimeVisibleParameterAnnotations',u1(1)+u2(1)+annotation()),
    attr('RuntimeInvisibleParameterAnnotations',u1(1)+u2(0)),
    attr('RuntimeVisibleTypeAnnotations',u2(1)+u1(0x14)+u1(0)+empty_annotation()),
    attr('AnnotationDefault',u1(ord('I'))+u2(value)),
    attr('Synthetic',b''), attr('Deprecated',b''),
]
method=u2(0x0001)+u2(test)+u2(desc)+u2(len(method_attrs))+b''.join(method_attrs)
field_name=utf('CONSTANT'); field_desc=utf('I')
field_attrs=[attr('ConstantValue',u2(value)),attr('Signature',u2(sig))]
field=u2(0x0019)+u2(field_name)+u2(field_desc)+u2(len(field_attrs))+b''.join(field_attrs)
class_attrs=[
    attr('InnerClasses',u2(1)+u2(this)+u2(parent)+u2(nested)+u2(0x0001)),
    attr('EnclosingMethod',u2(this)+u2(method_nat)),
    attr('BootstrapMethods',u2(0)),
    attr('RuntimeVisibleAnnotations',u2(1)+annotation()),
    attr('RuntimeInvisibleAnnotations',u2(0)),
    attr('RuntimeVisibleTypeAnnotations',u2(1)+u1(0x10)+u2(0xffff)+u1(0)+empty_annotation()),
    attr('SourceFile',u2(source)),
    attr('SourceDebugExtension',b'SMAP\nAttributeFixture.java\n'),
    attr('Signature',u2(sig)),
]
pool=b''.join(cp[1:])
result=(u4(0xcafebabe)+u2(0)+u2(52)+u2(len(cp))+pool+u2(0x0021)+u2(this)+u2(parent)+u2(0)+
        u2(1)+field+u2(1)+method+u2(len(class_attrs))+b''.join(class_attrs))
with open(sys.argv[1],'wb') as f: f.write(result)
