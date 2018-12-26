

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.io.Writer;
import java.util.logging.Level;
import java.util.logging.Logger;


public class HessianDebugOutputStream extends OutputStream {
    private OutputStream _os;

    private HessianDebugState _state;

    
    public HessianDebugOutputStream(OutputStream os, PrintWriter dbg) {
        _os = os;

        _state = new HessianDebugState(dbg);
    }

    
    public HessianDebugOutputStream(OutputStream os, Logger log, Level level) {
        this(os, new PrintWriter(new LogWriter(log, level)));
    }

    public void startTop2() {
        _state.startTop2();
    }

    
    public void write(int ch)
            throws IOException {
        ch = ch & 0xff;

        _os.write(ch);

        _state.next(ch);
    }

    public void flush()
            throws IOException {
        _os.flush();
    }

    
    public void close()
            throws IOException {
        OutputStream os = _os;
        _os = null;

        if (os != null)
            os.close();

        _state.println();
    }

    static class LogWriter extends Writer {
        private Logger _log;
        private Level _level;
        private StringBuilder _sb = new StringBuilder();

        LogWriter(Logger log, Level level) {
            _log = log;
            _level = level;
        }

        public void write(char ch) {
            if (ch == '\n' && _sb.length() > 0) {
                _log.log(_level, _sb.toString());
                _sb.setLength(0);
            } else
                _sb.append((char) ch);
        }

        public void write(char[] buffer, int offset, int length) {
            for (int i = 0; i < length; i++) {
                char ch = buffer[offset + i];

                if (ch == '\n' && _sb.length() > 0) {
                    _log.log(_level, _sb.toString());
                    _sb.setLength(0);
                } else
                    _sb.append((char) ch);
            }
        }

        public void flush() {
        }

        public void close() {
        }
    }
}
