package com.alibaba.json.test.benchmark.decode;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.serializer.SerializerFeature;
import com.alibaba.json.test.benchmark.BenchmarkCase;
import com.alibaba.json.test.codec.Codec;

import data.media.MediaContent;

public class EishayDecodeByClassName extends BenchmarkCase {

    private final String text;

    public EishayDecodeByClassName(){
        super("EishayDecode");

        text = JSON.toJSONString(EishayDecodeBytes.instance.getContent(), SerializerFeature.WriteEnumUsingToString,
                                 SerializerFeature.SortField, SerializerFeature.WriteClassName);
        System.out.println(text);
        System.out.println();

                    }

    @Override
    public void execute(Codec codec) throws Exception {
        MediaContent content = codec.decodeObject(text, MediaContent.class);
        if (content == null) {
            throw new Exception();
        }
    }

}
