
package com.alibaba.dubbo.common.json;


@Deprecated
public class Yylex {

    
    public static final int YYEOF = -1;
    
    public static final int STR2 = 4;
    public static final int STR1 = 2;
    public static final int YYINITIAL = 0;
    
    private static final int ZZ_BUFFERSIZE = 16384;
    
    private static final int ZZ_LEXSTATE[] = {
            0, 0, 1, 1, 2, 2
    };

    
    private static final String ZZ_CMAP_PACKED =
            "\11\0\1\13\1\13\2\0\1\13\22\0\1\13\1\0\1\10\1\0" +
                    "\1\2\2\0\1\11\3\0\1\7\1\43\1\4\1\5\1\14\12\1" +
                    "\1\44\6\0\1\33\3\3\1\6\1\32\5\2\1\34\1\2\1\36" +
                    "\3\2\1\25\1\35\1\24\1\26\5\2\1\41\1\12\1\42\1\0" +
                    "\1\2\1\0\1\27\1\15\2\3\1\23\1\16\5\2\1\30\1\2" +
                    "\1\17\3\2\1\20\1\31\1\21\1\22\5\2\1\37\1\0\1\40" +
                    "\uff82\0";

    
    private static final char[] ZZ_CMAP = zzUnpackCMap(ZZ_CMAP_PACKED);
    private static final String ZZ_ACTION_PACKED_0 =
            "\3\0\1\1\1\2\1\3\1\1\1\4\1\5\1\6" +
                    "\6\3\1\7\1\10\1\11\1\12\1\13\1\14\1\15" +
                    "\1\16\1\0\1\15\3\0\6\3\1\17\1\20\1\21" +
                    "\1\22\1\23\1\24\1\25\1\26\1\0\1\27\2\30" +
                    "\1\0\6\3\1\0\1\3\1\31\1\32\1\3\1\0" +
                    "\1\33\1\0\1\34";
    
    private static final int[] ZZ_ACTION = zzUnpackAction();
    private static final String ZZ_ROWMAP_PACKED_0 =
            "\0\0\0\45\0\112\0\157\0\224\0\271\0\336\0\157" +
                    "\0\157\0\u0103\0\u0128\0\u014d\0\u0172\0\u0197\0\u01bc\0\u01e1" +
                    "\0\157\0\157\0\157\0\157\0\157\0\157\0\u0206\0\157" +
                    "\0\u022b\0\u0250\0\u0275\0\u029a\0\u02bf\0\u02e4\0\u0309\0\u032e" +
                    "\0\u0353\0\u0378\0\u039d\0\157\0\157\0\157\0\157\0\157" +
                    "\0\157\0\157\0\157\0\u03c2\0\157\0\u03e7\0\u040c\0\u040c" +
                    "\0\u0431\0\u0456\0\u047b\0\u04a0\0\u04c5\0\u04ea\0\u050f\0\u0534" +
                    "\0\271\0\271\0\u0559\0\u057e\0\271\0\u05a3\0\157";
    
    private static final int[] ZZ_ROWMAP = zzUnpackRowMap();
    
    private static final int ZZ_TRANS[] = {
            3, 4, 5, 5, 6, 3, 5, 3, 7, 8,
            3, 9, 3, 5, 10, 11, 5, 12, 5, 5,
            13, 5, 5, 5, 5, 5, 14, 5, 5, 5,
            15, 16, 17, 18, 19, 20, 21, 22, 22, 22,
            22, 22, 22, 22, 22, 23, 22, 24, 22, 22,
            22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
            22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
            22, 22, 22, 22, 25, 25, 25, 25, 25, 25,
            25, 25, 25, 23, 26, 25, 25, 25, 25, 25,
            25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
            25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
            25, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, 4,
            -1, -1, -1, 27, 28, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, 28, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, 5, 5, 5, -1,
            -1, 5, -1, -1, -1, -1, -1, -1, 5, 5,
            5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            5, 5, 5, 5, 5, 5, -1, -1, -1, -1,
            -1, -1, -1, 4, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            9, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, 5, 5, 5,
            -1, -1, 5, -1, -1, -1, -1, -1, -1, 5,
            5, 5, 5, 5, 5, 5, 5, 5, 5, 29,
            5, 5, 5, 5, 5, 5, 5, -1, -1, -1,
            -1, -1, -1, -1, 5, 5, 5, -1, -1, 5,
            -1, -1, -1, -1, -1, -1, 5, 5, 5, 5,
            5, 30, 5, 5, 5, 5, 5, 5, 5, 5,
            5, 5, 5, 5, -1, -1, -1, -1, -1, -1,
            -1, 5, 5, 5, -1, -1, 5, -1, -1, -1,
            -1, -1, -1, 5, 5, 5, 31, 5, 5, 5,
            5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            5, -1, -1, -1, -1, -1, -1, -1, 5, 5,
            5, -1, -1, 5, -1, -1, -1, -1, -1, -1,
            5, 5, 5, 5, 5, 5, 5, 5, 32, 5,
            5, 5, 5, 5, 5, 5, 5, 5, -1, -1,
            -1, -1, -1, -1, -1, 5, 5, 5, -1, -1,
            5, -1, -1, -1, -1, -1, -1, 5, 5, 5,
            5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            5, 33, 5, 5, 5, -1, -1, -1, -1, -1,
            -1, -1, 5, 5, 5, -1, -1, 5, -1, -1,
            -1, -1, -1, -1, 5, 5, 5, 5, 5, 5,
            5, 5, 5, 34, 5, 5, 5, 5, 5, 5,
            5, 5, -1, -1, -1, -1, -1, -1, 22, 22,
            22, 22, 22, 22, 22, 22, -1, 22, -1, 22,
            22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
            22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
            22, 22, 22, 22, 22, -1, -1, -1, -1, -1,
            -1, -1, -1, 35, -1, 36, -1, 37, 38, 39,
            40, 41, 42, 43, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, 25, 25, 25, 25, 25, 25, 25, 25,
            25, -1, -1, 25, 25, 25, 25, 25, 25, 25,
            25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
            25, 25, 25, 25, 25, 25, 25, 25, 25, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, 44, 36,
            -1, 37, 38, 39, 40, 41, 42, 43, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, 45, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, 46, -1, -1, 47, -1, -1,
            47, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, 5, 5, 5, -1, -1, 5, -1, -1, -1,
            -1, -1, -1, 5, 5, 5, 5, 5, 5, 5,
            5, 5, 5, 5, 48, 5, 5, 5, 5, 5,
            5, -1, -1, -1, -1, -1, -1, -1, 5, 5,
            5, -1, -1, 5, -1, -1, -1, -1, -1, -1,
            5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            5, 49, 5, 5, 5, 5, 5, 5, -1, -1,
            -1, -1, -1, -1, -1, 5, 5, 5, -1, -1,
            5, -1, -1, -1, -1, -1, -1, 5, 5, 5,
            5, 5, 50, 5, 5, 5, 5, 5, 5, 5,
            5, 5, 5, 5, 5, -1, -1, -1, -1, -1,
            -1, -1, 5, 5, 5, -1, -1, 5, -1, -1,
            -1, -1, -1, -1, 5, 5, 5, 5, 5, 5,
            5, 5, 5, 51, 5, 5, 5, 5, 5, 5,
            5, 5, -1, -1, -1, -1, -1, -1, -1, 5,
            5, 5, -1, -1, 5, -1, -1, -1, -1, -1,
            -1, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            5, 5, 5, 5, 5, 5, 52, 5, 5, -1,
            -1, -1, -1, -1, -1, -1, 5, 5, 5, -1,
            -1, 5, -1, -1, -1, -1, -1, -1, 5, 5,
            5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            5, 5, 5, 53, 5, 5, -1, -1, -1, -1,
            -1, -1, -1, 54, -1, 54, -1, -1, 54, -1,
            -1, -1, -1, -1, -1, 54, 54, -1, -1, -1,
            -1, 54, -1, -1, -1, 54, -1, -1, 54, 54,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            45, -1, -1, -1, -1, 28, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, 28, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, 46, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, 5, 5, 5, -1, -1, 5,
            -1, -1, -1, -1, -1, -1, 5, 5, 5, 5,
            5, 5, 5, 5, 5, 5, 5, 5, 55, 5,
            5, 5, 5, 5, -1, -1, -1, -1, -1, -1,
            -1, 5, 5, 5, -1, -1, 5, -1, -1, -1,
            -1, -1, -1, 5, 5, 5, 5, 5, 5, 5,
            5, 5, 5, 5, 56, 5, 5, 5, 5, 5,
            5, -1, -1, -1, -1, -1, -1, -1, 5, 5,
            5, -1, -1, 5, -1, -1, -1, -1, -1, -1,
            5, 5, 5, 5, 5, 5, 57, 5, 5, 5,
            5, 5, 5, 5, 5, 5, 5, 5, -1, -1,
            -1, -1, -1, -1, -1, 5, 5, 5, -1, -1,
            57, -1, -1, -1, -1, -1, -1, 5, 5, 5,
            5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            5, 5, 5, 5, 5, -1, -1, -1, -1, -1,
            -1, -1, 5, 5, 5, -1, -1, 5, -1, -1,
            -1, -1, -1, -1, 5, 5, 5, 5, 5, 5,
            5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            58, 5, -1, -1, -1, -1, -1, -1, -1, 5,
            5, 5, -1, -1, 5, -1, -1, -1, -1, -1,
            -1, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            5, 5, 5, 5, 5, 5, 56, 5, 5, -1,
            -1, -1, -1, -1, -1, -1, 59, -1, 59, -1,
            -1, 59, -1, -1, -1, -1, -1, -1, 59, 59,
            -1, -1, -1, -1, 59, -1, -1, -1, 59, -1,
            -1, 59, 59, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, 5, 5, 5, -1, -1, 5, -1,
            -1, -1, -1, -1, -1, 5, 5, 5, 5, 5,
            5, 60, 5, 5, 5, 5, 5, 5, 5, 5,
            5, 5, 5, -1, -1, -1, -1, -1, -1, -1,
            5, 5, 5, -1, -1, 60, -1, -1, -1, -1,
            -1, -1, 5, 5, 5, 5, 5, 5, 5, 5,
            5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            -1, -1, -1, -1, -1, -1, -1, 61, -1, 61,
            -1, -1, 61, -1, -1, -1, -1, -1, -1, 61,
            61, -1, -1, -1, -1, 61, -1, -1, -1, 61,
            -1, -1, 61, 61, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, 62, -1, 62, -1, -1, 62,
            -1, -1, -1, -1, -1, -1, 62, 62, -1, -1,
            -1, -1, 62, -1, -1, -1, 62, -1, -1, 62,
            62, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    };
    
    private static final int ZZ_UNKNOWN_ERROR = 0;
    private static final int ZZ_NO_MATCH = 1;
    private static final int ZZ_PUSHBACK_2BIG = 2;
    
    private static final String ZZ_ERROR_MSG[] = {
            "Unkown internal scanner error",
            "Error: could not match input",
            "Error: pushback value was too large"
    };
    private static final String ZZ_ATTRIBUTE_PACKED_0 =
            "\3\0\1\11\3\1\2\11\7\1\6\11\1\1\1\11" +
                    "\1\0\1\1\3\0\6\1\10\11\1\0\1\11\2\1" +
                    "\1\0\6\1\1\0\4\1\1\0\1\1\1\0\1\11";
    
    private static final int[] ZZ_ATTRIBUTE = zzUnpackAttribute();
    
    private java.io.Reader zzReader;
    
    private int zzState;
    
    private int zzLexicalState = YYINITIAL;
    
    private char zzBuffer[] = new char[ZZ_BUFFERSIZE];
    
    private int zzMarkedPos;
    
    private int zzCurrentPos;
    
    private int zzStartRead;
    
    private int zzEndRead;
    
    private boolean zzAtEOF;
    
    
  
    private StringBuffer sb;

    
    Yylex(java.io.Reader in) {
        this.zzReader = in;
    }

    
    Yylex(java.io.InputStream in) {
        this(new java.io.InputStreamReader(in));
    }

    private static int[] zzUnpackAction() {
        int[] result = new int[63];
        int offset = 0;
        offset = zzUnpackAction(ZZ_ACTION_PACKED_0, offset, result);
        return result;
    }

    private static int zzUnpackAction(String packed, int offset, int[] result) {
        int i = 0;       
        int j = offset;  
        int l = packed.length();
        while (i < l) {
            int count = packed.charAt(i++);
            int value = packed.charAt(i++);
            do result[j++] = value; while (--count > 0);
        }
        return j;
    }

    
    
    
    
    
    
    
        private static int[] zzUnpackRowMap() {
        int[] result = new int[63];
        int offset = 0;
        offset = zzUnpackRowMap(ZZ_ROWMAP_PACKED_0, offset, result);
        return result;
    }

    private static int zzUnpackRowMap(String packed, int offset, int[] result) {
        int i = 0;  
        int j = offset;  
        int l = packed.length();
        while (i < l) {
            int high = packed.charAt(i++) << 16;
            result[j++] = high | packed.charAt(i++);
        }
        return j;
    }

    private static int[] zzUnpackAttribute() {
        int[] result = new int[63];
        int offset = 0;
        offset = zzUnpackAttribute(ZZ_ATTRIBUTE_PACKED_0, offset, result);
        return result;
    }

    private static int zzUnpackAttribute(String packed, int offset, int[] result) {
        int i = 0;       
        int j = offset;  
        int l = packed.length();
        while (i < l) {
            int count = packed.charAt(i++);
            int value = packed.charAt(i++);
            do result[j++] = value; while (--count > 0);
        }
        return j;
    }

    
    private static char[] zzUnpackCMap(String packed) {
        char[] map = new char[0x10000];
        int i = 0;  
        int j = 0;  
        while (i < 122) {
            int count = packed.charAt(i++);
            char value = packed.charAt(i++);
            do map[j++] = value; while (--count > 0);
        }
        return map;
    }


    
    private boolean zzRefill() throws java.io.IOException {

    
        if (zzStartRead > 0) {
            System.arraycopy(zzBuffer, zzStartRead,
                    zzBuffer, 0,
                    zzEndRead - zzStartRead);

      
            zzEndRead -= zzStartRead;
            zzCurrentPos -= zzStartRead;
            zzMarkedPos -= zzStartRead;
            zzStartRead = 0;
        }

    
        if (zzCurrentPos >= zzBuffer.length) {
      
            char newBuffer[] = new char[zzCurrentPos * 2];
            System.arraycopy(zzBuffer, 0, newBuffer, 0, zzBuffer.length);
            zzBuffer = newBuffer;
        }

    
        int numRead = zzReader.read(zzBuffer, zzEndRead,
                zzBuffer.length - zzEndRead);

        if (numRead > 0) {
            zzEndRead += numRead;
            return false;
        }
                if (numRead == 0) {
            int c = zzReader.read();
            if (c == -1) {
                return true;
            } else {
                zzBuffer[zzEndRead++] = (char) c;
                return false;
            }
        }

                return true;
    }


    
    public final void yyclose() throws java.io.IOException {
        zzAtEOF = true;            
        zzEndRead = zzStartRead;  

        if (zzReader != null)
            zzReader.close();
    }


    
    public final void yyreset(java.io.Reader reader) {
        zzReader = reader;
                zzAtEOF = false;
                zzEndRead = zzStartRead = 0;
        zzCurrentPos = zzMarkedPos = 0;
                zzLexicalState = YYINITIAL;
    }


    
    public final int yystate() {
        return zzLexicalState;
    }


    
    public final void yybegin(int newState) {
        zzLexicalState = newState;
    }


    
    public final String yytext() {
        return new String(zzBuffer, zzStartRead, zzMarkedPos - zzStartRead);
    }


    
    public final char yycharat(int pos) {
        return zzBuffer[zzStartRead + pos];
    }


    
    public final int yylength() {
        return zzMarkedPos - zzStartRead;
    }


    
    private void zzScanError(int errorCode) {
        String message;
        try {
            message = ZZ_ERROR_MSG[errorCode];
        } catch (ArrayIndexOutOfBoundsException e) {
            message = ZZ_ERROR_MSG[ZZ_UNKNOWN_ERROR];
        }

        throw new Error(message);
    }


    
    public void yypushback(int number) {
        if (number > yylength())
            zzScanError(ZZ_PUSHBACK_2BIG);

        zzMarkedPos -= number;
    }


    
    public JSONToken yylex() throws java.io.IOException, ParseException {
        int zzInput;
        int zzAction;

                int zzCurrentPosL;
        int zzMarkedPosL;
        int zzEndReadL = zzEndRead;
        char[] zzBufferL = zzBuffer;
        char[] zzCMapL = ZZ_CMAP;

        int[] zzTransL = ZZ_TRANS;
        int[] zzRowMapL = ZZ_ROWMAP;
        int[] zzAttrL = ZZ_ATTRIBUTE;

        while (true) {
            zzMarkedPosL = zzMarkedPos;

            zzAction = -1;

            zzCurrentPosL = zzCurrentPos = zzStartRead = zzMarkedPosL;

            zzState = ZZ_LEXSTATE[zzLexicalState];


            zzForAction:
            {
                while (true) {

                    if (zzCurrentPosL < zzEndReadL)
                        zzInput = zzBufferL[zzCurrentPosL++];
                    else if (zzAtEOF) {
                        zzInput = YYEOF;
                        break zzForAction;
                    } else {
                                                zzCurrentPos = zzCurrentPosL;
                        zzMarkedPos = zzMarkedPosL;
                        boolean eof = zzRefill();
                                                zzCurrentPosL = zzCurrentPos;
                        zzMarkedPosL = zzMarkedPos;
                        zzBufferL = zzBuffer;
                        zzEndReadL = zzEndRead;
                        if (eof) {
                            zzInput = YYEOF;
                            break zzForAction;
                        } else {
                            zzInput = zzBufferL[zzCurrentPosL++];
                        }
                    }
                    int zzNext = zzTransL[zzRowMapL[zzState] + zzCMapL[zzInput]];
                    if (zzNext == -1) break zzForAction;
                    zzState = zzNext;

                    int zzAttributes = zzAttrL[zzState];
                    if ((zzAttributes & 1) == 1) {
                        zzAction = zzState;
                        zzMarkedPosL = zzCurrentPosL;
                        if ((zzAttributes & 8) == 8) break zzForAction;
                    }

                }
            }

                        zzMarkedPos = zzMarkedPosL;

            switch (zzAction < 0 ? zzAction : ZZ_ACTION[zzAction]) {
                case 25: {
                    return new JSONToken(JSONToken.NULL, null);
                }
                case 29:
                    break;
                case 13: {
                    sb.append(yytext());
                }
                case 30:
                    break;
                case 18: {
                    sb.append('\b');
                }
                case 31:
                    break;
                case 9: {
                    return new JSONToken(JSONToken.LSQUARE);
                }
                case 32:
                    break;
                case 2: {
                    Long val = Long.valueOf(yytext());
                    return new JSONToken(JSONToken.INT, val);
                }
                case 33:
                    break;
                case 16: {
                    sb.append('\\');
                }
                case 34:
                    break;
                case 8: {
                    return new JSONToken(JSONToken.RBRACE);
                }
                case 35:
                    break;
                case 26: {
                    return new JSONToken(JSONToken.BOOL, Boolean.TRUE);
                }
                case 36:
                    break;
                case 23: {
                    sb.append('\'');
                }
                case 37:
                    break;
                case 5: {
                    sb = new StringBuffer();
                    yybegin(STR2);
                }
                case 38:
                    break;
                case 27: {
                    return new JSONToken(JSONToken.BOOL, Boolean.FALSE);
                }
                case 39:
                    break;
                case 12: {
                    return new JSONToken(JSONToken.COLON);
                }
                case 40:
                    break;
                case 21: {
                    sb.append('\r');
                }
                case 41:
                    break;
                case 3: {
                    return new JSONToken(JSONToken.IDENT, yytext());
                }
                case 42:
                    break;
                case 28: {
                    try {
                        sb.append((char) Integer.parseInt(yytext().substring(2), 16));
                    } catch (Exception e) {
                        throw new ParseException(e.getMessage());
                    }
                }
                case 43:
                    break;
                case 10: {
                    return new JSONToken(JSONToken.RSQUARE);
                }
                case 44:
                    break;
                case 17: {
                    sb.append('/');
                }
                case 45:
                    break;
                case 11: {
                    return new JSONToken(JSONToken.COMMA);
                }
                case 46:
                    break;
                case 15: {
                    sb.append('"');
                }
                case 47:
                    break;
                case 24: {
                    Double val = Double.valueOf(yytext());
                    return new JSONToken(JSONToken.FLOAT, val);
                }
                case 48:
                    break;
                case 1: {
                    throw new ParseException("Unexpected char [" + yytext() + "]");
                }
                case 49:
                    break;
                case 19: {
                    sb.append('\f');
                }
                case 50:
                    break;
                case 7: {
                    return new JSONToken(JSONToken.LBRACE);
                }
                case 51:
                    break;
                case 14: {
                    yybegin(YYINITIAL);
                    return new JSONToken(JSONToken.STRING, sb.toString());
                }
                case 52:
                    break;
                case 22: {
                    sb.append('\t');
                }
                case 53:
                    break;
                case 4: {
                    sb = new StringBuffer();
                    yybegin(STR1);
                }
                case 54:
                    break;
                case 20: {
                    sb.append('\n');
                }
                case 55:
                    break;
                case 6: {
                }
                case 56:
                    break;
                default:
                    if (zzInput == YYEOF && zzStartRead == zzCurrentPos) {
                        zzAtEOF = true;
                        return null;
                    } else {
                        zzScanError(ZZ_NO_MATCH);
                    }
            }
        }
    }


}