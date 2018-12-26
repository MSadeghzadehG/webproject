

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;


abstract public class HessianEnvelope {
    
    abstract public Hessian2Output wrap(Hessian2Output out)
            throws IOException;

    
    abstract public Hessian2Input unwrap(Hessian2Input in)
            throws IOException;

    
    abstract public Hessian2Input unwrapHeaders(Hessian2Input in)
            throws IOException;
}
