

package org.elasticsearch.common;


import java.io.IOException;
import java.util.Base64;
import java.util.Random;

class RandomBasedUUIDGenerator implements UUIDGenerator {

    
    @Override
    public String getBase64UUID() {
        return getBase64UUID(SecureRandomHolder.INSTANCE);
    }

    
    public String getBase64UUID(Random random) {
        final byte[] randomBytes = new byte[16];
        random.nextBytes(randomBytes);
        
        randomBytes[6] &= 0x0f;  
        randomBytes[6] |= 0x40;  
        
        
        randomBytes[8] &= 0x3f;  
        randomBytes[8] |= 0x80;  
        return Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes);
    }
}
