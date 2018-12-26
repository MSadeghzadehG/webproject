
package com.alibaba.dubbo.common.serialize.kryo;

import com.alibaba.dubbo.common.logger.Logger;
import com.alibaba.dubbo.common.logger.LoggerFactory;
import com.alibaba.dubbo.common.serialize.kryo.utils.ReflectionUtils;

import com.esotericsoftware.kryo.Kryo;
import com.esotericsoftware.kryo.Serializer;
import com.esotericsoftware.kryo.serializers.JavaSerializer;

public class CompatibleKryo extends Kryo {

    private static final Logger logger = LoggerFactory.getLogger(CompatibleKryo.class);

    @Override
    public Serializer getDefaultSerializer(Class type) {
        if (type == null) {
            throw new IllegalArgumentException("type cannot be null.");
        }

        if (!type.isArray() && !type.isEnum() && !ReflectionUtils.checkZeroArgConstructor(type)) {
            if (logger.isWarnEnabled()) {
                logger.warn(type + " has no zero-arg constructor and this will affect the serialization performance");
            }
            return new JavaSerializer();
        }
        return super.getDefaultSerializer(type);
    }
}
