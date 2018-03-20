

package com.badlogic.gdx.tools;

import com.badlogic.gdx.utils.Array;

import java.io.File;
import java.io.FilenameFilter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.LinkedHashMap;
import java.util.regex.Pattern;


public class FileProcessor {
	FilenameFilter inputFilter;
	Comparator<File> comparator = new Comparator<File>() {
		public int compare (File o1, File o2) {
			return o1.getName().compareTo(o2.getName());
		}
	};
	Array<Pattern> inputRegex = new Array();
	String outputSuffix;
	ArrayList<Entry> outputFiles = new ArrayList();
	boolean recursive = true;
	boolean flattenOutput;

	Comparator<Entry> entryComparator = new Comparator<Entry>() {
		public int compare (Entry o1, Entry o2) {
			return comparator.compare(o1.inputFile, o2.inputFile);
		}
	};

	public FileProcessor () {
	}

	
	public FileProcessor (FileProcessor processor) {
		inputFilter = processor.inputFilter;
		comparator = processor.comparator;
		inputRegex.addAll(processor.inputRegex);
		outputSuffix = processor.outputSuffix;
		recursive = processor.recursive;
		flattenOutput = processor.flattenOutput;
	}

	public FileProcessor setInputFilter (FilenameFilter inputFilter) {
		this.inputFilter = inputFilter;
		return this;
	}

	
	public FileProcessor setComparator (Comparator<File> comparator) {
		this.comparator = comparator;
		return this;
	}

	
	public FileProcessor addInputSuffix (String... suffixes) {
		for (String suffix : suffixes)
			addInputRegex("(?i).*" + Pattern.quote(suffix));
		return this;
	}

	public FileProcessor addInputRegex (String... regexes) {
		for (String regex : regexes)
			inputRegex.add(Pattern.compile(regex));
		return this;
	}

	
	public FileProcessor setOutputSuffix (String outputSuffix) {
		this.outputSuffix = outputSuffix;
		return this;
	}

	public FileProcessor setFlattenOutput (boolean flattenOutput) {
		this.flattenOutput = flattenOutput;
		return this;
	}

	
	public FileProcessor setRecursive (boolean recursive) {
		this.recursive = recursive;
		return this;
	}

	
	public ArrayList<Entry> process (String inputFileOrDir, String outputRoot) throws Exception {
		return process(new File(inputFileOrDir), outputRoot == null ? null : new File(outputRoot));
	}

	
	public ArrayList<Entry> process (File inputFileOrDir, File outputRoot) throws Exception {
		if (!inputFileOrDir.exists()) throw new IllegalArgumentException("Input file does not exist: " + inputFileOrDir.getAbsolutePath());
		if (inputFileOrDir.isFile())
			return process(new File[] {inputFileOrDir}, outputRoot);
		else
			return process(inputFileOrDir.listFiles(), outputRoot);
	}

	
	public ArrayList<Entry> process (File[] files, File outputRoot) throws Exception {
		if (outputRoot == null) outputRoot = new File("");
		outputFiles.clear();

		LinkedHashMap<File, ArrayList<Entry>> dirToEntries = new LinkedHashMap();
		process(files, outputRoot, outputRoot, dirToEntries, 0);

		ArrayList<Entry> allEntries = new ArrayList();
		for (java.util.Map.Entry<File, ArrayList<Entry>> mapEntry : dirToEntries.entrySet()) {
			ArrayList<Entry> dirEntries = mapEntry.getValue();
			if (comparator != null) Collections.sort(dirEntries, entryComparator);

			File inputDir = mapEntry.getKey();
			File newOutputDir = null;
			if (flattenOutput)
				newOutputDir = outputRoot;
			else if (!dirEntries.isEmpty()) 				newOutputDir = dirEntries.get(0).outputDir;
			String outputName = inputDir.getName();
			if (outputSuffix != null) outputName = outputName.replaceAll("(.*)\\..*", "$1") + outputSuffix;

			Entry entry = new Entry();
			entry.inputFile = mapEntry.getKey();
			entry.outputDir = newOutputDir;
			if (newOutputDir != null)
				entry.outputFile = newOutputDir.length() == 0 ? new File(outputName) : new File(newOutputDir, outputName);

			try {
				processDir(entry, dirEntries);
			} catch (Exception ex) {
				throw new Exception("Error processing directory: " + entry.inputFile.getAbsolutePath(), ex);
			}
			allEntries.addAll(dirEntries);
		}

		if (comparator != null) Collections.sort(allEntries, entryComparator);
		for (Entry entry : allEntries) {
			try {
				processFile(entry);
			} catch (Exception ex) {
				throw new Exception("Error processing file: " + entry.inputFile.getAbsolutePath(), ex);
			}
		}

		return outputFiles;
	}

	private void process (File[] files, File outputRoot, File outputDir, LinkedHashMap<File, ArrayList<Entry>> dirToEntries,
		int depth) {
				for (File file : files) {
			File dir = file.getParentFile();
			ArrayList<Entry> entries = dirToEntries.get(dir);
			if (entries == null) {
				entries = new ArrayList();
				dirToEntries.put(dir, entries);
			}
		}

		for (File file : files) {
			if (file.isFile()) {
				if (inputRegex.size > 0) {
					boolean found = false;
					for (Pattern pattern : inputRegex) {
						if (pattern.matcher(file.getName()).matches()) {
							found = true;
							continue;
						}
					}
					if (!found) continue;
				}

				File dir = file.getParentFile();
				if (inputFilter != null && !inputFilter.accept(dir, file.getName())) continue;

				String outputName = file.getName();
				if (outputSuffix != null) outputName = outputName.replaceAll("(.*)\\..*", "$1") + outputSuffix;

				Entry entry = new Entry();
				entry.depth = depth;
				entry.inputFile = file;
				entry.outputDir = outputDir;

				if (flattenOutput) {
					entry.outputFile = new File(outputRoot, outputName);
				} else {
					entry.outputFile = new File(outputDir, outputName);
				}

				dirToEntries.get(dir).add(entry);
			}
			if (recursive && file.isDirectory()) {
				File subdir = outputDir.getPath().length() == 0 ? new File(file.getName()) : new File(outputDir, file.getName());
				process(file.listFiles(inputFilter), outputRoot, subdir, dirToEntries, depth + 1);
			}
		}
	}

	
	protected void processFile (Entry entry) throws Exception {
	}

	
	protected void processDir (Entry entryDir, ArrayList<Entry> files) throws Exception {
	}

	
	protected void addProcessedFile (Entry entry) {
		outputFiles.add(entry);
	}

	
	static public class Entry {
		public File inputFile;
		
		public File outputDir;
		public File outputFile;
		public int depth;

		public Entry () {
		}

		public Entry (File inputFile, File outputFile) {
			this.inputFile = inputFile;
			this.outputFile = outputFile;
		}

		public String toString () {
			return inputFile.toString();
		}
	}
}
