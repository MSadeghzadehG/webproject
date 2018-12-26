
package com.alibaba.fastjson.asm;


public class Type {
    
    public static final Type VOID_TYPE    = new Type(0, null, ('V' << 24) | (5 << 16) | (0 << 8) | 0, 1);

    
    public static final Type BOOLEAN_TYPE = new Type(1, null, ('Z' << 24) | (0 << 16) | (5 << 8) | 1, 1);

    
    public static final Type CHAR_TYPE    = new Type(2, null, ('C' << 24) | (0 << 16) | (6 << 8) | 1, 1);

    
    public static final Type BYTE_TYPE    = new Type(3, null, ('B' << 24) | (0 << 16) | (5 << 8) | 1, 1);

    
    public static final Type SHORT_TYPE   = new Type(4, null, ('S' << 24) | (0 << 16) | (7 << 8) | 1, 1);

    
    public static final Type INT_TYPE     = new Type(5, null, ('I' << 24) | (0 << 16) | (0 << 8) | 1, 1);

    
    public static final Type FLOAT_TYPE   = new Type(6, null, ('F' << 24) | (2 << 16) | (2 << 8) | 1, 1);

    
    public static final Type LONG_TYPE    = new Type(7, null, ('J' << 24) | (1 << 16) | (1 << 8) | 2, 1);

    
    public static final Type DOUBLE_TYPE  = new Type(8, null, ('D' << 24) | (3 << 16) | (3 << 8) | 2, 1);

            
    
    protected final int        sort;

    
    private final char[]     buf;

    
    private final int        off;

    
    private final int        len;

            
    private Type(final int sort, final char[] buf, final int off, final int len){
        this.sort = sort;
        this.buf = buf;
        this.off = off;
        this.len = len;
    }

    
    public static Type getType(final String typeDescriptor) {
        return getType(typeDescriptor.toCharArray(), 0);
    }

    public static int getArgumentsAndReturnSizes(final String desc) {
        int n = 1;
        int c = 1;
        while (true) {
            char car = desc.charAt(c++);
            if (car == ')') {
                car = desc.charAt(c);
                return n << 2 | (car == 'V' ? 0 : (car == 'D' || car == 'J' ? 2 : 1));
            } else if (car == 'L') {
                while (desc.charAt(c++) != ';') {
                }
                n += 1;
            } else if (car == 'D' || car == 'J') {
                n += 2;
            } else {
                n += 1;
            }
        }
    }

    
    private static Type getType(final char[] buf, final int off) {
        int len;
        switch (buf[off]) {
            case 'V':
                return VOID_TYPE;
            case 'Z':
                return BOOLEAN_TYPE;
            case 'C':
                return CHAR_TYPE;
            case 'B':
                return BYTE_TYPE;
            case 'S':
                return SHORT_TYPE;
            case 'I':
                return INT_TYPE;
            case 'F':
                return FLOAT_TYPE;
            case 'J':
                return LONG_TYPE;
            case 'D':
                return DOUBLE_TYPE;
            case '[':
                len = 1;
                while (buf[off + len] == '[') {
                    ++len;
                }
                if (buf[off + len] == 'L') {
                    ++len;
                    while (buf[off + len] != ';') {
                        ++len;
                    }
                }
                return new Type(9 , buf, off, len + 1);
                            default:
                len = 1;
                while (buf[off + len] != ';') {
                    ++len;
                }
                return new Type(10, buf, off + 1, len - 1);
        }
    }

    public String getInternalName() {
        return new String(buf, off, len);
    }

            
    
    String getDescriptor() {
        return new String(this.buf, off, len);
    }

    private int getDimensions() {
        int i = 1;
        while (buf[off + i] == '[') {
            ++i;
        }
        return i;
    }

    static Type[] getArgumentTypes(final String methodDescriptor) {
        char[] buf = methodDescriptor.toCharArray();
        int off = 1;
        int size = 0;
        for (;;) {
            char car = buf[off++];
            if (car == ')') {
                break;
            } else if (car == 'L') {
                while (buf[off++] != ';') {
                }
                ++size;
            } else if (car != '[') {
                ++size;
            }
        }

        Type[] args = new Type[size];
        off = 1;
        size = 0;
        while (buf[off] != ')') {
            args[size] = getType(buf, off);
            off += args[size].len + (args[size].sort == 10  ? 2 : 0);
            size += 1;
        }
        return args;
    }

    protected String getClassName() {
        switch (sort) {
            case 0:                 return "void";
            case 1:                 return "boolean";
            case 2:                 return "char";
            case 3:                 return "byte";
            case 4:                 return "short";
            case 5:                 return "int";
            case 6:                 return "float";
            case 7:                 return "long";
            case 8:                 return "double";
            case 9:                 Type elementType = getType(buf, off + getDimensions());
                StringBuffer b = new StringBuffer(elementType.getClassName());
                for (int i = getDimensions(); i > 0; --i) {
                    b.append("[]");
                }
                return b.toString();
                        default:
                return new String(buf, off, len).replace('/', '.');
        }
    }
}
