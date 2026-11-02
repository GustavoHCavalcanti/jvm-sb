#!/usr/bin/env python3
"""Build a tiny structurally valid Java 8 ClassFile fixture without a JDK."""
import struct
import sys

u1 = lambda n: struct.pack(">B", n)
u2 = lambda n: struct.pack(">H", n)
u4 = lambda n: struct.pack(">I", n)
def utf8(value):
    encoded = value.encode("ascii")
    return u1(1) + u2(len(encoded)) + encoded

major = int(sys.argv[2]) if len(sys.argv) > 2 and sys.argv[2] != "--root" else 52
root = "--root" in sys.argv[2:]
class_name = "java/lang/Object" if root else "Minimal"
this_class = 4 if root else 2
super_class = 0 if root else 4
data = (u4(0xCAFEBABE) + u2(0) + u2(major) + u2(8) +
        utf8(class_name) + u1(7) + u2(1) +
        utf8("java/lang/Object") + u1(7) + u2(3) +
        utf8("test") + utf8("()I") + utf8("Code") +
        u2(0x0021) + u2(this_class) + u2(super_class) + u2(0) + u2(0) + u2(1) +
        u2(0x0009) + u2(5) + u2(6) + u2(1) + u2(7) + u4(14) +
        u2(1) + u2(0) + u4(2) + u1(4) + u1(0xac) + u2(0) + u2(0) + u2(0))
with open(sys.argv[1], "wb") as output:
    output.write(data)
