

package java.io;

public class FileWriter {

	private final RandomAccessFile file;

	public FileWriter (String name) throws FileNotFoundException {
		this.file = new RandomAccessFile(new File(name), "rw");
	}

	public void close () throws IOException {
		file.close();
	}

	public void flush () throws IOException {
		file.flush();
	}

	public void write (String s) throws IOException {
		for (int i = 0; i < s.length(); i++) {
			file.write(s.charAt(i));
		}
	}


}
