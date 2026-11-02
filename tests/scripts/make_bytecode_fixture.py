#!/usr/bin/env python3
"""Generate bytecode decoder fixture for switch and wide operands."""
import struct
import sys
u1=lambda x: struct.pack('>B',x & 0xff)
u2=lambda x: struct.pack('>H',x & 0xffff)
u4=lambda x: struct.pack('>I',x & 0xffffffff)
cp=[None]
def utf(s):
    b=s.encode('ascii'); cp.append(u1(1)+u2(len(b))+b); return len(cp)-1
def cls(name):
    n=utf(name);cp.append(u1(7)+u2(n));return len(cp)-1
this=cls('BytecodeFixture');parent=cls('java/lang/Object')
code_name=utf('Code'); descriptor=utf('()V')
table_name=utf('tableSwitch'); lookup_name=utf('lookupSwitch'); wide_name=utf('wideLoad'); int_desc=utf('()I')
switch=bytes([0xaa,0,0,0])+u4(24)+u4(-1)+u4(0)+u4(24)+u4(24)+bytes([0xb1])
lookup=bytes([0xab,0,0,0])+u4(20)+u4(1)+u4(7)+u4(20)+bytes([0xb1])
wide=bytes([0xc4,0x15,0x01,0x2c,0xac])
def code_attr(body,stack=1,locals_=301):
    info=u2(stack)+u2(locals_)+u4(len(body))+body+u2(0)+u2(0)
    return u2(code_name)+u4(len(info))+info
def method(name,desc,body,stack=1,locals_=301):
    return u2(0x0009)+u2(name)+u2(desc)+u2(1)+code_attr(body,stack,locals_)
methods=[method(table_name,descriptor,switch),method(lookup_name,descriptor,lookup),method(wide_name,int_desc,wide)]
pool=b''.join(cp[1:])
data=u4(0xcafebabe)+u2(0)+u2(52)+u2(len(cp))+pool+u2(0x0021)+u2(this)+u2(parent)+u2(0)+u2(0)+u2(len(methods))+b''.join(methods)+u2(0)
with open(sys.argv[1],'wb') as f:f.write(data)
