
package com.alibaba.dubbo.qos.server.handler;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelPipeline;
import io.netty.handler.codec.ByteToMessageDecoder;
import io.netty.handler.codec.LineBasedFrameDecoder;
import io.netty.handler.codec.http.HttpObjectAggregator;
import io.netty.handler.codec.http.HttpServerCodec;
import io.netty.handler.codec.string.StringDecoder;
import io.netty.handler.codec.string.StringEncoder;
import io.netty.handler.timeout.IdleStateHandler;
import io.netty.util.CharsetUtil;
import io.netty.util.concurrent.ScheduledFuture;

import java.util.List;
import java.util.concurrent.TimeUnit;

public class QosProcessHandler extends ByteToMessageDecoder {

    private ScheduledFuture<?> welcomeFuture;

    private String welcome;
        private boolean acceptForeignIp;

    public static String prompt = "dubbo>";

    public QosProcessHandler(String welcome, boolean acceptForeignIp) {
        this.welcome = welcome;
        this.acceptForeignIp = acceptForeignIp;
    }

    @Override
    public void channelActive(final ChannelHandlerContext ctx) throws Exception {
        welcomeFuture = ctx.executor().schedule(new Runnable() {

            @Override
            public void run() {
                if (welcome != null) {
                    ctx.write(Unpooled.wrappedBuffer(welcome.getBytes()));
                    ctx.writeAndFlush(Unpooled.wrappedBuffer(prompt.getBytes()));
                }
            }

        }, 500, TimeUnit.MILLISECONDS);
    }

    @Override
    protected void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) throws Exception {
        if (in.readableBytes() < 1) {
            return;
        }

                final int magic = in.getByte(in.readerIndex());

        ChannelPipeline p = ctx.pipeline();
        p.addLast(new LocalHostPermitHandler(acceptForeignIp));
        if (isHttp(magic)) {
                        if (welcomeFuture != null && welcomeFuture.isCancellable()) {
                welcomeFuture.cancel(false);
            }
            p.addLast(new HttpServerCodec());
            p.addLast(new HttpObjectAggregator(1048576));
            p.addLast(new HttpProcessHandler());
            p.remove(this);
        } else {
            p.addLast(new LineBasedFrameDecoder(2048));
            p.addLast(new StringDecoder(CharsetUtil.UTF_8));
            p.addLast(new StringEncoder(CharsetUtil.UTF_8));
            p.addLast(new IdleStateHandler(0, 0, 5 * 60));
            p.addLast(new TelnetProcessHandler());
            p.remove(this);
        }
    }

        private static boolean isHttp(int magic) {
        return magic == 'G' || magic == 'P';
    }
}
