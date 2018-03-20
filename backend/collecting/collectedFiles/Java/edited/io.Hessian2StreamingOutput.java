

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;
import java.io.OutputStream;


public class Hessian2StreamingOutput {
    private Hessian2Output _out;

    
    public Hessian2StreamingOutput(OutputStream os) {
        _out = new Hessian2Output(os);
    }

    public boolean isCloseStreamOnClose() {
        return _out.isCloseStreamOnClose();
    }

    public void setCloseStreamOnClose(boolean isClose) {
        _out.setCloseStreamOnClose(isClose);
    }

    
    public void writeObject(Object object)
            throws IOException {
        _out.writeStreamingObject(object);
    }

    
    public void flush()
            throws IOException {
        _out.flush();
    }

    
    public void close()
            throws IOException {
        _out.close();
    }
}
