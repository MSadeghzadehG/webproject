

package com.badlogic.gdx.backends.lwjgl3.audio;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import org.lwjgl.BufferUtils;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.StreamUtils;
import com.jcraft.jogg.Packet;
import com.jcraft.jogg.Page;
import com.jcraft.jogg.StreamState;
import com.jcraft.jogg.SyncState;
import com.jcraft.jorbis.Block;
import com.jcraft.jorbis.Comment;
import com.jcraft.jorbis.DspState;
import com.jcraft.jorbis.Info;


public class OggInputStream extends InputStream {
	private final static int BUFFER_SIZE = 512;
	
	
	private int convsize = BUFFER_SIZE * 4;
	
	private byte[] convbuffer;
	
	private InputStream input;
	
	private Info oggInfo = new Info(); 	
	private boolean endOfStream;

	
	private SyncState syncState = new SyncState(); 	
	private StreamState streamState = new StreamState(); 	
	private Page page = new Page(); 	
	private Packet packet = new Packet(); 
	
	private Comment comment = new Comment(); 	
	private DspState dspState = new DspState(); 	
	private Block vorbisBlock = new Block(dspState); 
	
	byte[] buffer;
	
	int bytes = 0;
	
	boolean bigEndian = ByteOrder.nativeOrder().equals(ByteOrder.BIG_ENDIAN);
	
	boolean endOfBitStream = true;
	
	boolean inited = false;

	
	private int readIndex;
	
	private ByteBuffer pcmBuffer;
	
	private int total;

	
	public OggInputStream (InputStream input) {
		this(input, null);
	}

	
	OggInputStream (InputStream input, OggInputStream previousStream) {
		if (previousStream == null) {
			convbuffer = new byte[convsize];
			pcmBuffer = BufferUtils.createByteBuffer(4096 * 500);
		} else {
			convbuffer = previousStream.convbuffer;
			pcmBuffer = previousStream.pcmBuffer;
		}

		this.input = input;
		try {
			total = input.available();
		} catch (IOException ex) {
			throw new GdxRuntimeException(ex);
		}

		init();
	}

	
	public int getLength () {
		return total;
	}

	public int getChannels () {
		return oggInfo.channels;
	}

	public int getSampleRate () {
		return oggInfo.rate;
	}

	
	private void init () {
		initVorbis();
		readPCM();
	}

	
	public int available () {
		return endOfStream ? 0 : 1;
	}

	
	private void initVorbis () {
		syncState.init();
	}

	
	private boolean getPageAndPacket () {
								
				int index = syncState.buffer(BUFFER_SIZE);
		if (index == -1) return false;

		buffer = syncState.data;
		if (buffer == null) {
			endOfStream = true;
			return false;
		}

		try {
			bytes = input.read(buffer, index, BUFFER_SIZE);
		} catch (Exception e) {
			throw new GdxRuntimeException("Failure reading Vorbis.", e);
		}
		syncState.wrote(bytes);

				if (syncState.pageout(page) != 1) {
						if (bytes < BUFFER_SIZE) return false;

						throw new GdxRuntimeException("Input does not appear to be an Ogg bitstream.");
		}

						streamState.init(page.serialno());

				
								
		oggInfo.init();
		comment.init();
		if (streamState.pagein(page) < 0) {
						throw new GdxRuntimeException("Error reading first page of Ogg bitstream.");
		}

		if (streamState.packetout(packet) != 1) {
						throw new GdxRuntimeException("Error reading initial header packet.");
		}

		if (oggInfo.synthesis_headerin(comment, packet) < 0) {
						throw new GdxRuntimeException("Ogg bitstream does not contain Vorbis audio data.");
		}

						
										
		int i = 0;
		while (i < 2) {
			while (i < 2) {
				int result = syncState.pageout(page);
				if (result == 0) break; 								
				if (result == 1) {
					streamState.pagein(page); 															while (i < 2) {
						result = streamState.packetout(packet);
						if (result == 0) break;
						if (result == -1) {
																					throw new GdxRuntimeException("Corrupt secondary header.");
						}

						oggInfo.synthesis_headerin(comment, packet);
						i++;
					}
				}
			}
						index = syncState.buffer(BUFFER_SIZE);
			if (index == -1) return false;
			buffer = syncState.data;
			try {
				bytes = input.read(buffer, index, BUFFER_SIZE);
			} catch (Exception e) {
				throw new GdxRuntimeException("Failed to read Vorbis.", e);
			}
			if (bytes == 0 && i < 2) {
				throw new GdxRuntimeException("End of file before finding all Vorbis headers.");
			}
			syncState.wrote(bytes);
		}

		convsize = BUFFER_SIZE / oggInfo.channels;

						dspState.synthesis_init(oggInfo); 		vorbisBlock.init(dspState); 								
		return true;
	}

	
	private void readPCM () {
		boolean wrote = false;

		while (true) { 			if (endOfBitStream) {
				if (!getPageAndPacket()) {
					break;
				}
				endOfBitStream = false;
			}

			if (!inited) {
				inited = true;
				return;
			}

			float[][][] _pcm = new float[1][][];
			int[] _index = new int[oggInfo.channels];
						while (!endOfBitStream) {
				while (!endOfBitStream) {
					int result = syncState.pageout(page);

					if (result == 0) {
						break; 					}

					if (result == -1) { 												Gdx.app.log("gdx-audio", "Error reading OGG: Corrupt or missing data in bitstream.");
					} else {
						streamState.pagein(page); 												while (true) {
							result = streamState.packetout(packet);

							if (result == 0) break; 							if (result == -1) { 															} else {
																int samples;
								if (vorbisBlock.synthesis(packet) == 0) { 									dspState.synthesis_blockin(vorbisBlock);
								}

																																
								while ((samples = dspState.synthesis_pcmout(_pcm, _index)) > 0) {
									float[][] pcm = _pcm[0];
																		int bout = (samples < convsize ? samples : convsize);

																											for (int i = 0; i < oggInfo.channels; i++) {
										int ptr = i * 2;
																				int mono = _index[i];
										for (int j = 0; j < bout; j++) {
											int val = (int)(pcm[i][mono + j] * 32767.);
																						if (val > 32767) {
												val = 32767;
											}
											if (val < -32768) {
												val = -32768;
											}
											if (val < 0) val = val | 0x8000;

											if (bigEndian) {
												convbuffer[ptr] = (byte)(val >>> 8);
												convbuffer[ptr + 1] = (byte)(val);
											} else {
												convbuffer[ptr] = (byte)(val);
												convbuffer[ptr + 1] = (byte)(val >>> 8);
											}
											ptr += 2 * (oggInfo.channels);
										}
									}

									int bytesToWrite = 2 * oggInfo.channels * bout;
									if (bytesToWrite > pcmBuffer.remaining()) {
										throw new GdxRuntimeException("Ogg block too big to be buffered: " + bytesToWrite + " :: " + pcmBuffer.remaining());
									} else {
										pcmBuffer.put(convbuffer, 0, bytesToWrite);
									}

									wrote = true;
									dspState.synthesis_read(bout); 																										}
							}
						}
						if (page.eos() != 0) {
							endOfBitStream = true;
						}

						if ((!endOfBitStream) && (wrote)) {
							return;
						}
					}
				}

				if (!endOfBitStream) {
					bytes = 0;
					int index = syncState.buffer(BUFFER_SIZE);
					if (index >= 0) {
						buffer = syncState.data;
						try {
							bytes = input.read(buffer, index, BUFFER_SIZE);
						} catch (Exception e) {
							throw new GdxRuntimeException("Error during Vorbis decoding.", e);
						}
					} else {
						bytes = 0;
					}
					syncState.wrote(bytes);
					if (bytes == 0) {
						endOfBitStream = true;
					}
				}
			}

									streamState.clear();

						
			vorbisBlock.clear();
			dspState.clear();
			oggInfo.clear(); 		}

				syncState.clear();
		endOfStream = true;
	}

	public int read () {
		if (readIndex >= pcmBuffer.position()) {
			pcmBuffer.clear();
			readPCM();
			readIndex = 0;
		}
		if (readIndex >= pcmBuffer.position()) {
			return -1;
		}

		int value = pcmBuffer.get(readIndex);
		if (value < 0) {
			value = 256 + value;
		}
		readIndex++;

		return value;
	}

	public boolean atEnd () {
		return endOfStream && (readIndex >= pcmBuffer.position());
	}

	public int read (byte[] b, int off, int len) {
		for (int i = 0; i < len; i++) {
			int value = read();
			if (value >= 0) {
				b[i] = (byte)value;
			} else {
				if (i == 0) {
					return -1;
				} else {
					return i;
				}
			}
		}

		return len;
	}

	public int read (byte[] b) {
		return read(b, 0, b.length);
	}

	public void close () {
		StreamUtils.closeQuietly(input);
	}
}
