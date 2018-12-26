

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;


public interface HessianRemoteResolver {
    
    public Object lookup(String type, String url)
            throws IOException;
}
