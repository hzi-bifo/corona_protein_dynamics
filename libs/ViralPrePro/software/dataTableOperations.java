import java.io.*;
import java.util.*;
import java.text.*;
import java.util.regex.*;

public class dataTableOperations {
	public static void buildDataTable (String input, String output, int index) {
		try {
			BufferedReader	br = null;
			BufferedWriter	bw = null;
			String	inpLine = null,
				newLine = System.getProperty("line.separator"),
				tmpSplit [] = null,
				lineSplit [] = null,
				tmpIsolate [] = null,
				tmpString = null,
				data [][] = null,
				dataNames [] = {"ID","gi","gb","epiFlu","Isolate","Segment","Host","Name","Type","SubType","Year","String","Sequence","Country","Date"};
			Matcher	m = null;
			Pattern	p = null;
			boolean found = false;
			Stack	dataStack = new Stack();
			int	numData = 0,
				usedInd = 0,
				op, cl, se;
			
			br = new BufferedReader (new FileReader (new File (input)));
			while ((inpLine=br.readLine()) != null) {
				// start of new sequence
				if (inpLine.length() == 0 || inpLine.charAt(0) == ' ') continue;
				if (inpLine.charAt(0) == '>') {
					if (p != null) {
						tmpIsolate [12] = tmpIsolate[12].toUpperCase();
						dataStack.push(tmpIsolate);
					}
					tmpIsolate = new String [dataNames.length];
					tmpIsolate [0] = "f0dp" + numData++;
					tmpIsolate [11] = inpLine.substring(1,inpLine.length());
					tmpIsolate [12] = "";
					// epiFlu database?
					p = Pattern.compile("EPI_ISL_[0-9]*");
					m = p.matcher(inpLine);
					found = m.find();
					if (found) {
						lineSplit = inpLine.substring(1,inpLine.length()).split(" \\| ");
						if (lineSplit.length == 1) continue;
						tmpIsolate [4] = m.group();
						tmpIsolate [3] = lineSplit[0];
						tmpIsolate [5] = lineSplit[1];
						tmpIsolate [7] = lineSplit[2].replace(' ','_');
						tmpIsolate [2] = lineSplit[4];
						tmpIsolate [9] = lineSplit[5];
						tmpSplit = lineSplit[2].split("\\/");
						tmpIsolate [8] = tmpSplit[0];
						// date
						if (tmpSplit[tmpSplit.length-1].length() < 2) usedInd = tmpSplit.length-2;
						else usedInd = tmpSplit.length-1;
						p = Pattern.compile("[0-9]{2,4}");
						m = p.matcher(tmpSplit[usedInd]);
						if (m.find() == false) tmpIsolate [10] = "NA";
						else {
							tmpString = m.group();
							if (tmpString.length() == 2) tmpIsolate [10] = (Integer.parseInt(tmpString) < 18 ? "20" : "19") + tmpString;
							else tmpIsolate [10] = tmpString;
						}
						// exact date (new)

						if (lineSplit.length > 6) {
							tmpIsolate [14] = lineSplit [6].replaceAll("-","/");
							tmpIsolate [10] = tmpIsolate[14].substring(0,tmpIsolate[14].indexOf('/'));
							tmpIsolate [10] = tmpIsolate[14].substring(0,4);
						}
						// host
						if (tmpSplit[1].indexOf("Human") > -1 || tmpSplit[1].indexOf("human") > -1 || tmpSplit[1].indexOf("HUMAN") > -1) tmpIsolate [6] = "human";
						else {
							if (usedInd > 3) tmpIsolate [6] = tmpSplit [1].replace(' ','_').toLowerCase();
							else tmpIsolate [6] = "human";
						}
					}
					else {
						// ncbi database
						p = Pattern.compile ("[A-Z]{1,3}[0-9]{4,7}");
						m = p.matcher(inpLine);
						found = m.find();
						if (found) {
							lineSplit = inpLine.substring(1,inpLine.length()).split("\\|");
							if (lineSplit.length == 1) continue;
							tmpIsolate [2] = m.group();
							if (inpLine.substring(1,3).equals("gi")) tmpIsolate [1] = lineSplit [1];
							tmpSplit = lineSplit[lineSplit.length-1].split("\\/",-1);
							tmpIsolate [6] = tmpSplit[1].toLowerCase();
							tmpIsolate [9] = tmpSplit[3];
							tmpIsolate [13] = tmpSplit[5];
							tmpIsolate [14] = formatNCBIdate(tmpSplit[6],tmpSplit[7],tmpSplit[8]);
							tmpIsolate [10] = tmpSplit[6];
							if (tmpSplit[2].indexOf("(") > -1) tmpIsolate [5] = tmpSplit[2].substring(tmpSplit[2].indexOf('(')+1,tmpSplit[2].indexOf(')'));
							else tmpIsolate [5] = tmpSplit[2];
							tmpString = lineSplit[lineSplit.length-1];
							tmpIsolate [8] = ""+tmpString.charAt(tmpString.indexOf("Influenza")+10);
							usedInd = tmpString.indexOf("Influenza");
							op = tmpString.indexOf('(',usedInd);
							se = tmpString.indexOf('(',op+1);
							cl = tmpString.indexOf(')',usedInd);
							tmpIsolate [7] = tmpString.substring(op+1,(se > -1 ? (se < cl ? se : cl) : cl));
						}
					}
				}
				else {
					tmpIsolate [12] += inpLine;
				}
			}
			dataStack.push(tmpIsolate);
			
			data = new String [numData][];
			for (int i = numData -1; i >= 0; i--) {
				data [i] = (String [])dataStack.pop();
			}
			
			if (index != -1) data = sortDataTable(data,index);
			
			bw = new BufferedWriter (new FileWriter (new File (output)));
			bw.write("#"+numData+newLine);
			for (int i = 0; i < dataNames.length; i++) bw.write(dataNames[i] + "\t");
			bw.write(newLine);
			for (int i = 0; i < data.length; i++) {
				for (int j = 0; j < data[i].length; j++) bw.write((data[i][j] == null ? "" : data[i][j]) + "\t");
				bw.write(newLine);
			}
			
			br.close();
			bw.close();
		}
		catch (IOException e) {
			System.out.println("Caught the following exception: " + e);
			System.out.println("Please check your input parameters!");
			System.exit(1);
		}
	}
	
	public static String formatNCBIdate (String year, String month, String day) {
		String myDate = year + "/";
		if (month.length() == 0) myDate += "00/";
		else myDate += month + "/";
		if (day.length() == 0) myDate += "00/";
		else myDate += day + "/";
		return myDate;
	}
	
	public static String [][] readFasta (String fileName) {
		try {
			BufferedReader	br = null;
			String	inpLine = null,
				tmpSequence [] = null,
				data [][] = null;
			Stack	dataStack = new Stack();
			int	numData = 0;
			
			br = new BufferedReader (new FileReader (new File (fileName)));
			while ((inpLine=br.readLine()) != null) {
				// start of new sequence
				if (inpLine.length() == 0 || inpLine.charAt(0) == ' ') continue;
				if (inpLine.charAt(0) == '>') {
					if (tmpSequence != null) {
						dataStack.push(tmpSequence);
					}
					numData++;
					tmpSequence = new String [2];
					tmpSequence [0] = inpLine.substring(1,inpLine.length());
					tmpSequence [1] = "";
				}
				else {
					tmpSequence [1] += inpLine;
				}
			}
			dataStack.push(tmpSequence);
			br.close();
			
			data = new String [numData][];
			for (int i = numData -1; i >= 0; i--) {
				data [i] = (String [])dataStack.pop();
			}
			
			return data;
		}
		catch (IOException e) {
			System.out.println("Caught the following exception: " + e);
			System.out.println("Please check your input parameters!");
			System.exit(1);
		}
		
		return null;
	}
	
	public static String [][] readDuplicatesFile (String fileName) {
		try {
			BufferedReader	br = null;
			String	inpLine = null,
				tmpSequence [] = null,
				data [][] = null;
			Stack	dataStack = new Stack();
			int	numData = 0;
			
			br = new BufferedReader (new FileReader (new File (fileName)));
			while ((inpLine=br.readLine()) != null) {
				tmpSequence = inpLine.split(" ",-1);
				dataStack.push(tmpSequence);
				numData++;
			}
			br.close();
			
			data = new String [numData][];
			for (int i = numData -1; i >= 0; i--) {
				data [i] = (String [])dataStack.pop();
			}
			
			return data;
		}
		catch (IOException e) {
			System.out.println("Caught the following exception: " + e);
			System.out.println("Please check your input parameters!");
			System.exit(1);
		}
		
		return null;
	}
	
	public static void enterAlignment (String dataTab_fileName, String alignment_fileName, String duplicates_fileName) {
		String	dataTab [][] = readDataTable (dataTab_fileName),
			alignment [][] = readFasta (alignment_fileName),
			duplicates [][] = (duplicates_fileName == null ? null : readDuplicatesFile (duplicates_fileName)),
			tmpSplit [] = null;
		int	tmpVal;
		
		if (duplicates_fileName != null) {
			for (int i = 0; i < duplicates.length; i++) {
				tmpVal = Integer.parseInt (duplicates [i][0]);
				dataTab [tmpVal][12] = alignment [i][1];
				if (duplicates [i].length > 1) {
					tmpSplit = duplicates [i][1].split(",",-1);
					for (int j = 0; j < tmpSplit.length; j++) dataTab [Integer.parseInt(tmpSplit[j])][12] = dataTab [tmpVal][12];
				}
			}
		}
		else {
			for (int i = 0; i < alignment.length; i++) {
				for (int j = 0; j < dataTab.length; j++) {
					if (alignment [i][0].equals(dataTab [j][0])) {
						dataTab [j][12] = alignment [i][1];
					}
				}
			}
		}
		
		try {
			BufferedWriter	bw = new BufferedWriter (new FileWriter (new File (dataTab_fileName + ".withAln")));
			String	newLine = System.getProperty("line.separator"),
				header = header = "ID\tgi\tgb\tepiFlu\tIsolate\tSegment\tHost\tName\tType\tSubType\tYear\tString\tSequence\tCountry\tDate";
			
			bw.write ("#" + dataTab.length + newLine + header + newLine);
			for (int i = 0; i < dataTab.length; i++) {
				for (int j = 0; j < dataTab [i].length; j++) bw.write (dataTab [i][j] + "\t");
				bw.write (newLine);
			}
			bw.close();
		}
		catch (IOException e) {
			System.out.println("Caught the following exception: " + e);
			System.out.println("Please check your input parameters!");
			System.exit(1);
		}
	}
	
	public static void writeSortedData (String dataTab_fileName, int index) {
		String	dataTab [][] = sortDataTable(readDataTable (dataTab_fileName),index);
		
		try {
			BufferedWriter	bw = new BufferedWriter (new FileWriter (new File (dataTab_fileName + ".sorted")));
			String	newLine = System.getProperty("line.separator"),
				header = header = "ID\tgi\tgb\tepiFlu\tIsolate\tSegment\tHost\tName\tType\tSubType\tYear\tString\tSequence\tCountry\tDate";
			
			bw.write ("#" + dataTab.length + newLine + header + newLine);
			for (int i = 0; i < dataTab.length; i++) {
				for (int j = 0; j < dataTab [i].length; j++) bw.write (dataTab [i][j] + "\t");
				bw.write (newLine);
			}
			bw.close();
		}
		catch (IOException e) {
			System.out.println("Caught the following exception: " + e);
			System.out.println("Please check your input parameters!");
			System.exit(1);
		}
	}
	
	public static String [][] readDataTable (String fileName) {
		try {
			BufferedReader	br = null;
			String	inpLine = null,
				dataTable [][] = null;
			int	numData = 0,
				index = 0;
			
			br = new BufferedReader (new FileReader (new File (fileName)));
			while ((inpLine=br.readLine()) != null) {
				// skip empty lines
				if (inpLine.length() == 0) continue;
				// read first line, skip second one
				if (inpLine.charAt(0) == '#') {
						numData = Integer.parseInt(inpLine.substring(1,inpLine.length()));
						dataTable = new String [numData][];
						inpLine = br.readLine();
				}
				else {
					dataTable [index] = inpLine.split("\t",-1);
					index++;
				}
			}
			br.close();
			return dataTable;
		}
		catch (IOException e) {
			System.out.println("Caught the following exception: " + e);
			System.out.println("Please check your input parameters!");
			System.exit(1);
		}
		return null;
	}
	
	public static String [][] sortDataTable (String [][] dataTab, int index) {
		int	indices [] = new int [dataTab.length],
			help,
			data [] = new int [dataTab.length];
		boolean	isInt = false;
		
		if (dataTab[0][index].length() == 0) return dataTab;
		
		try {
			Integer.parseInt (dataTab[0][index]);
			isInt = true;
		}
		catch (NumberFormatException e) {}
		
		if (isInt) {
			for (int i = 0; i < data.length; i++) data [i] = Integer.parseInt (dataTab[i][index]);
		}
		else {
			String	tmpSplit [] = null;
			for (int i = 0; i < data.length; i++) {
				tmpSplit = dataTab[i][index].split("\\/",-1);
				data [i] = Integer.parseInt (tmpSplit[0] + tmpSplit[1] + tmpSplit[2]);
			}
		}
		for (int i = 0; i < indices.length; i++) indices [i] = i;
		
		for (int i = indices.length-1; i > 0; i--) {
			for (int j = 0; j < i; j++) {
				if (data[indices[j]] > data[indices[j+1]]) {
					help = indices [j];
					indices [j] = indices [j+1];
					indices [j+1] = help;
				}
			}
		}
		
		String	resArray [][] = new String [dataTab.length][];
		for (int i = 0; i < dataTab.length; i++) resArray [i] = dataTab [indices[i]];
		
		return resArray;
	}
	
	public static boolean isAmbigues (char a, String type) {
		if (type.equals("dna")) {
			switch (a) {
				case 'A': case 'a': case 'C': case 'c': case 'G': case 'g': case 'T': case 't': case '-': return false;
				default: return true;
			}
		}
		else {
			switch (a) {
				case 'B': case 'b': case 'Z': case 'z': case 'X': case 'x': case '?': return true;
				default: return false;
			}
		}
	}
	
	public static Set<String> resolveAmbiguity (char a, String type) {
		Set<String>	A = new HashSet<String>();
		
		if (type.equals("dna")) {
			switch (a) {
				case 'R': case 'r': A.add("A"); A.add("G"); break;
				case 'Y': case 'y': A.add("C"); A.add("T"); break;
				case 'M': case 'm': A.add("A"); A.add("C"); break;
				case 'K': case 'k': A.add("G"); A.add("T"); break;
				case 'S': case 's': A.add("C"); A.add("G"); break;
				case 'W': case 'w': A.add("A"); A.add("T"); break;
				case 'H': case 'h': A.add("A"); A.add("C"); A.add("T"); break;
				case 'B': case 'b': A.add("C"); A.add("G"); A.add("T"); break;
				case 'V': case 'v': A.add("A"); A.add("C"); A.add("G"); break;
				case 'D': case 'd': A.add("A"); A.add("G"); A.add("T"); break;
				case 'N': case 'n': case '?': A.add("A"); A.add("C"); A.add("G"); A.add("T"); break; // if (gaps) A.add("-");
				default: A.add(""+Character.toUpperCase(a)); break;
			}
		}
		else {
			switch (a) {
				case 'B': case 'b': A.add("D"); A.add("N"); break;
				case 'Z': case 'z': A.add("E"); A.add("Q"); break;
				case 'X': case 'x': case '?': for (int i = 65; i < 91; i++) { A.add("" + (char)i); } break; //if (gaps) A.add("-"); 
				default: A.add(""+Character.toUpperCase(a)); break;
			}
		}
		
		return A;
	}
	
	public static boolean checkAmbiguity (char a, char b, String type) {
		if (!isAmbigues(a,type) && !isAmbigues(b,type)) return false;
		
		Set<String>	A = resolveAmbiguity(a,type),
				B = resolveAmbiguity(b,type);
		
		A.retainAll(B);	// intersection
		
		return !A.isEmpty ();
	}
	
	public static void removeIdenticals (String file, int sortIndex, int index, boolean gaps, String type) {
		String	[][] tmpData = readDataTable (file);
// 		String	[][] dataTab = tmpData;//sortDataTable (tmpData,sortIndex);
		String	[][] dataTab = sortDataTable (tmpData,sortIndex);
		String	[] duplicateArray = new String [dataTab.length];
		boolean	unique [] = new boolean [dataTab.length],
			set = false;
		int	numIdent = 0,
			start = 0,
			end = 0;
		char	tmpSplit [][] = new char [dataTab.length][],
			tmpSplit_a [] = null,
			tmpSplit_b [] = null;
		
		for (int i = 0; i < unique.length; i++) {
			unique [i] = true;
			tmpSplit [i] = dataTab [i][index].toCharArray();
		}
		
		for (int i = 0; i < dataTab.length; i++) {
			if (!unique[i]) continue;
			duplicateArray [i] = "" + dataTab[i][0] + " ";
			
			for (int j = i+1; j < dataTab.length; j++) {
				if (!unique[j]) continue;
				set = false;
				if (dataTab[i][index].equals(dataTab[j][index])) {
					set = true;
				}
				else {
					if (dataTab[i][index].length() == dataTab[j][index].length()) {
						// this only works for alignments!!!
						set = true;
						for (int k = 0; k < tmpSplit [i].length; k++) {
							if (tmpSplit [i][k] != tmpSplit [j][k] && (gaps ? tmpSplit [j][k] != '-' : true) && (type == null ? true : !checkAmbiguity (tmpSplit [i][k],tmpSplit [j][k], type))) {
								set = false;
								break;
							}
						}
					}
				}
				
				if (set) {
					if (duplicateArray[i].charAt(duplicateArray[i].length()-1) != ' ') duplicateArray [i] += ",";
					duplicateArray [i] += dataTab[j][0];
					unique [j] = false;
					numIdent++;
				}
			}
		}
		//if (unique[dataTab.length-1]) duplicateArray [dataTab.length-1] = "" + (dataTab.length-1) + " ";
		
		try {
			String	newLine = System.getProperty("line.separator"),
				header = "ID\tgi\tgb\tepiFlu\tIsolate\tSegment\tHost\tName\tType\tSubType\tYear\tString\tSequence\tCountry\tDate";
			BufferedWriter	bw = new BufferedWriter (new FileWriter (new File (file+".noIdents")));
			BufferedWriter	bw_dups = new BufferedWriter (new FileWriter (new File (file+".duplicates")));
			bw.write ("#"+(dataTab.length-numIdent)+newLine);
			bw.write (header+newLine);
			
			for (int i = 0; i < dataTab.length; i++) {
				if (unique[i]) {
					for (int j = 0; j < dataTab[i].length; j++) bw.write(dataTab[i][j] + "\t");
					bw.write (newLine);
					bw_dups.write (duplicateArray [i] + newLine);
				}
			}
			bw.close();
			bw_dups.close();
		}
		catch (IOException e) {
			System.out.println("Caught the following exception: " + e);
			System.out.println("Please check your input parameters!");
			System.exit(1);
		}
	}
	
	public static void reduceSNPdata (String file, int index) {
		String	dataTab [][] = readDataTable (file),
			duplicateArray [] = new String [dataTab.length],
			tmpString = "";
		boolean	unique [] = new boolean [dataTab.length],
			uniqueSite [] = new boolean [dataTab [0][index].length()];
		int	numIdent = 0,
			start = 0,
			end = 0;
		for (int i = 0; i < unique.length; i++) unique [i] = true;
		
		// remove seqs without any SNP
		duplicateArray [0] = "" + dataTab[0][0] + " ";
		for (int i = 1; i < dataTab.length; i++) {
			start = Math.min(Math.min(dataTab[i][index].indexOf('A'),dataTab[i][index].indexOf('G')),Math.min(dataTab[i][index].indexOf('C'),dataTab[i][index].indexOf('T')));
			end = Math.max(Math.max(dataTab[i][index].lastIndexOf('A'),dataTab[i][index].lastIndexOf('G')),Math.max(dataTab[i][index].lastIndexOf('C'),dataTab[i][index].lastIndexOf('T')));
			
			if (dataTab[0][index].substring(start,end+1).equals(dataTab[i][index].substring(start,end+1))) {
				if (duplicateArray[0].charAt(duplicateArray[0].length()-1) != ' ') duplicateArray[0] += ",";
				duplicateArray[0] += dataTab[i][0];
				unique [i] = false;
				numIdent++;
			}
		}
		
		// get sites with SNPS
		for (int i = 0; i < dataTab [0][index].length(); i++) {
			uniqueSite [i] = true;
			for (int j = 1; j < dataTab.length; j++) {
				if ((dataTab [j][index].charAt (i) != dataTab [0][index].charAt (i)) && (dataTab [j][index].charAt (i) != '?')) {
					uniqueSite [i] = false;
					break;
				}
			}
		}
		
		// reduce Sequence to non-unique sites
		for (int i = 0; i < dataTab.length; i++) {
			tmpString = new String ("");
			for (int j = 0; j < uniqueSite.length; j++) {
				if (!uniqueSite [j]) tmpString += dataTab [i][index].charAt (j);
			}
			dataTab [i][index] = new String (tmpString);
		}
		
		// get unique seqs (seqs with SNPs)
		for (int i = 1; i < (dataTab.length-1); i++) {
			if (!unique[i]) continue;
			duplicateArray [i] = "" + dataTab[i][0] + " ";
			for (int j = i+1; j < dataTab.length; j++) {
				if (!unique[j]) continue;
				if (dataTab[i][index].equals(dataTab[j][index])) {
					if (duplicateArray[i].charAt(duplicateArray[i].length()-1) != ' ') duplicateArray [i] += ",";
					duplicateArray [i] += dataTab[j][0];
					unique [j] = false;
					numIdent++;
				}
			}
		}
		if (unique[dataTab.length-1]) duplicateArray [dataTab.length-1] = "" + (dataTab.length-1) + " ";
		
		try {
			String	newLine = System.getProperty("line.separator"),
				header = "ID\tgi\tgb\tepiFlu\tIsolate\tSegment\tHost\tName\tType\tSubType\tYear\tString\tSequence\tCountry\tDate";
			BufferedWriter	bw = new BufferedWriter (new FileWriter (new File (file+".noIdents")));
			BufferedWriter	bw_dups = new BufferedWriter (new FileWriter (new File (file+".duplicates")));
			BufferedWriter	bw_seqs = new BufferedWriter (new FileWriter (new File (file+".fas")));
			bw.write ("#"+(dataTab.length-numIdent)+newLine);
			bw.write (header+newLine);
			
			bw_seqs.write (">f0dp0000" + newLine + dataTab [0][12] + newLine);
			for (int i = 0; i < dataTab.length; i++) {
				if (unique[i]) {
					for (int j = 0; j < dataTab[i].length; j++) bw.write(dataTab[i][j] + "\t");
					bw.write (newLine);
					bw_dups.write (duplicateArray [i] + newLine);
					bw_seqs.write (">" + dataTab [i][0] + newLine + dataTab [i][12] + newLine);
				}
			}
			bw.close();
			bw_dups.close();
			bw_seqs.close();
		}
		catch (IOException e) {
			System.out.println("Caught the following exception: " + e);
			System.out.println("Please check your input parameters!");
			System.exit(1);
		}
	}
	
	public static void checkDataTables (String fileA, String fileB) {
		try {
			BufferedWriter	bw = null;
			String	newLine = System.getProperty("line.separator"),
				header = "ID\tgi\tgb\tepiFlu\tIsolate\tSegment\tHost\tName\tType\tSubType\tYear\tString\tSequence\tCountry\tDate",
				dataA [][]= null,
				dataB [][] = null;
			int	orderA [] = null,
				orderB [] = null,
				numToUse = 0;
			boolean	usedB [] = null,
				selEpi,
				selNcbi,
				selString,
				use = false;
			
			dataA = readDataTable (fileA);
			dataB = readDataTable (fileB);
			
			orderA = new int [dataA.length];
			orderB = new int [dataB.length];
			usedB = new boolean [dataB.length];
			for (int i = 0; i < usedB.length; i++) usedB [i] = false;
			
			for (int i = 0; i < dataA.length; i++) {
				use = false;
				for (int j = 0; j < dataB.length; j++) {
					if (usedB [j]) continue;
					// epiFlu identifier
					selEpi = !dataA[i][3].equals("");
					selNcbi = !dataA[i][2].equals("");
					if (selEpi) {
						if (dataA[i][3].equals(dataB[j][3])) use = true;
					}
					else if (selNcbi) {
						if (dataA[i][2].equals(dataB[j][2])) use = true;
					}
					else {
						if (dataA[i][11].equals(dataB[j][11])) use = true;
					}
					if (use) {
						orderA [numToUse] = i;
						orderB [numToUse] = j;
						usedB[j] = true;
						numToUse++;
						break;
					}
				}
			}
			
			bw = new BufferedWriter (new FileWriter (new File (fileA+".mod")));
			bw.write ("#"+numToUse+newLine);
			bw.write (header+newLine);
			
			for (int i = 0; i < numToUse; i++) {
				for (int j = 0; j < dataA[orderA[i]].length; j++) bw.write(dataA[orderA[i]][j] + "\t");
				bw.write (newLine);
			}
			bw.close();
			
			bw = new BufferedWriter (new FileWriter (new File (fileB+".mod")));
			bw.write ("#"+numToUse+newLine);
			bw.write (header+newLine);
			
			for (int i = 0; i < numToUse; i++) {
				bw.write (dataA[orderA[i]][0] + "\t");
				for (int j = 1; j < dataB[orderB[i]].length; j++) bw.write(dataB[orderB[i]][j] + "\t");
				bw.write (newLine);
			}
			bw.close();
		}
		catch (IOException e) {
			System.out.println("Caught the following exception: " + e);
			System.out.println("Please check your input parameters!");
			System.exit(1);
		}
	}

	public static void mergeDataTables (String [] fileList) {
		try {
			BufferedReader	br = null;
			BufferedWriter	bw = null;
			String	inpLine = null,
				header = "ID\tgi\tgb\tepiFlu\tIsolate\tSegment\tHost\tName\tType\tSubType\tYear\tString\tSequence\tCountry\tDate",
				newLine = System.getProperty("line.separator");
			Stack	input = new Stack(),
				reverse = new Stack();
			int	numSeqs = 0,
				tmpNum = 0;
			
			for (int i = 0; i < (fileList.length-1); i++) {
				br = new BufferedReader (new FileReader (new File (fileList[i])));
				while ((inpLine=br.readLine()) != null) {
					// skip empty lines
					if (inpLine.length() == 0 || inpLine.charAt(0) == ' ') continue;
					// skip first two lines
					if (inpLine.charAt(0) == '#') {
						tmpNum = Integer.parseInt(inpLine.substring(1,inpLine.length()));
						if (tmpNum == 0) break;
						numSeqs += tmpNum;
						inpLine = br.readLine();
						inpLine = br.readLine();
					}
					input.push("f"+i+inpLine.substring(inpLine.indexOf("dp"),inpLine.length()));
				}
				br.close();
			}
			
			while (!input.empty()) reverse.push(input.pop());
			
			bw = new BufferedWriter (new FileWriter (new File (fileList[fileList.length-1])));
			bw.write("#"+numSeqs+newLine);
			bw.write(header+newLine);
			while (!reverse.empty()) bw.write(((String)reverse.pop()) + newLine);
			bw.close();
		}
		catch (IOException e) {
			System.out.println("Caught the following exception: " + e);
			System.out.println("Please check your input parameters!");
			System.exit(1);
		}
	}

	public static void saveMapping (String input, String output, String which) {
		try {
			BufferedReader	br = null;
			BufferedWriter	bw_fasta = null,
					bw_map = null;
			String	inpLine = null,
				newLine = System.getProperty("line.separator"),
				lineSplit [] = null,
				tmpSplit [] = null;
			
			bw_fasta = new BufferedWriter (new FileWriter (new File (output+".fasta")));
			bw_map = new BufferedWriter (new FileWriter (new File (output+".map")));
			
			br = new BufferedReader (new FileReader (new File (input)));
			while ((inpLine=br.readLine()) != null) {
				// skip empty lines
				if (inpLine.length() == 0 || inpLine.charAt(0) == ' ') continue;
				// skip first two lines
				if (inpLine.charAt(0) == '#') {
					inpLine = br.readLine();
					inpLine = br.readLine();
				}
				lineSplit = inpLine.split("\t",-1);
				if (which != null) {
					tmpSplit = lineSplit[14].split("\\/",-1);
					if (which.equals("m")) lineSplit[10] = tmpSplit[1];
				}
				bw_fasta.write(">"+lineSplit[0]+newLine+lineSplit[12]+newLine);
				bw_map.write(lineSplit[0]+"\t"+(lineSplit[3].length()==0 ? lineSplit[2] : lineSplit[3])+"\t"+lineSplit[7]+"\t"+lineSplit[9]+"\t"+lineSplit[10]+"\t"+lineSplit[6]+"\t"+lineSplit[11]+"\t"+lineSplit[14]+newLine);
			}
			br.close();
			bw_fasta.close();
			bw_map.close();
		}
		catch (IOException e) {
			System.out.println("Caught the following exception: " + e);
			System.out.println("Please check your input parameters!");
			System.exit(1);
		}
	}
	
	public static void saveMapping_differentTime (String input, String output, String which) {
		try {
			BufferedReader	br = null;
			BufferedWriter	bw_map = null;
			String	inpLine = null,
				newLine = System.getProperty("line.separator"),
				lineSplit [] = null,
				tmpSplit [] = null;
			
			bw_map = new BufferedWriter (new FileWriter (new File (output+".map")));
			
			br = new BufferedReader (new FileReader (new File (input)));
			while ((inpLine=br.readLine()) != null) {
				// skip empty lines
				if (inpLine.length() == 0 || inpLine.charAt(0) == ' ') continue;
				// skip first two lines
				if (inpLine.charAt(0) == '#') {
					inpLine = br.readLine();
					inpLine = br.readLine();
				}
				lineSplit = inpLine.split("\t",-1);
				if (which.equals("m")) {
					tmpSplit = lineSplit[14].split("\\/",-1);
					lineSplit [10] = tmpSplit [1];
				}
				bw_map.write(lineSplit[0]+"\t"+(lineSplit[3].length()==0 ? lineSplit[2] : lineSplit[3])+"\t"+lineSplit[7]+"\t"+lineSplit[9]+"\t"+lineSplit[10]+"\t"+lineSplit[6]+"\t"+lineSplit[11]+"\t"+lineSplit[14]+newLine);
			}
			br.close();
			bw_map.close();
		}
		catch (IOException e) {
			System.out.println("Caught the following exception: " + e);
			System.out.println("Please check your input parameters!");
			System.exit(1);
		}
	}
	
	public static void writeFastaFromDataTab (String input, String output, int index) {
		try {
			BufferedReader	br = null;
			BufferedWriter	bw = null;
			String	inpLine = null,
				newLine = System.getProperty("line.separator"),
				lineSplit [] = null;
			
			bw = new BufferedWriter (new FileWriter (new File (output)));
			br = new BufferedReader (new FileReader (new File (input)));
			
			while ((inpLine=br.readLine()) != null) {
				// skip empty lines
				if (inpLine.length() == 0 || inpLine.charAt(0) == ' ') continue;
				// skip first two lines
				if (inpLine.charAt(0) == '#') {
					inpLine = br.readLine();
					inpLine = br.readLine();
				}
				lineSplit = inpLine.split("\t",-1);
				bw.write(">"+(index == 0 ? lineSplit[11] : lineSplit[0])+newLine+lineSplit[12]+newLine);
			}
			br.close();
			bw.close();
		}
		catch (IOException e) {
			System.out.println("Caught the following exception: " + e);
			System.out.println("Please check your input parameters!");
			System.exit(1);
		}
	}
	
	private static int indexOf (String dataTable [][], String id, int column) {
		for (int i = 0; i < dataTable.length; i++) if (dataTable [i][column].equals(id)) return i;
		return -1;
	}
	
	public static void rarefactionTable (String dataTabFile, String duplicatesFile, String time) { // important dataTable should be sorted by date
		String	dataTable [][] = readDataTable (dataTabFile),
			duplicates [][] = readDuplicatesFile (duplicatesFile),
			tmpSplit [] = null;
		int	dates [][] = new int [dataTable.length][5],
			min = Integer.MAX_VALUE,
			max = Integer.MIN_VALUE,
			numIsolates [] = null,
			associationMap [][] = null,
			ind_i = 0,
			ind_j = 0;
		
		for (int i = 0; i < dataTable.length; i++) {
			tmpSplit = dataTable [i][14].split("/");
			dates [i][0] = Integer.parseInt(tmpSplit[0]);
			if (tmpSplit[1].length() > 0) dates [i][1] = Integer.parseInt(tmpSplit[1]);
			if (tmpSplit[2].length() > 0) dates [i][2] = Integer.parseInt(tmpSplit[2]);
			
			switch (time.charAt(0)) {
				case 'p': dates [i][3] = dates [i][0]; break;
				case 'm': dates [i][3] = dates [i][1]; break;
				case 'n': dates [i][3] = dates [i][0] + (dates [i][1] > 3 ? 1 : 0); break;
				case 'b': dates [i][3] = (dates [i][1] > 9 || dates [i][1] < 4? -1 : 1)*(dates [i][0] + (dates [i][1] > 9 ? 1 : 0)); break;
				//case 'b': dates [i][3] = (dates [i][1] > 9  || dates [i][1] < 4? -1 : 1)*(dates [i][0] + (dates [i][1] > 9 ? 1 : 0)); break;
				case 's': dates [i][3] = dates [i][0] + (dates [i][1] > 9 ? 1 : 0); break;
				default: dates [i][3] = dates [i][0]; break;
			}
			
			if (min > Math.abs(dates [i][3])) min = Math.abs(dates[i][3]);
			if (max < Math.abs(dates [i][3])) max = Math.abs(dates[i][3]);
		}
		
		numIsolates = new int [(max - min + 1)*(time.charAt(0) == 'b' ? 2 : 1)];
		associationMap = new int [duplicates.length][(max - min + 1)*(time.charAt(0) == 'b' ? 2 : 1)];
		
		for (int i = 0; i < duplicates.length; i++) {
			ind_i = indexOf (dataTable,duplicates [i][0],0);
			if (time.charAt(0) == 'b') {
				if (dates [ind_i][3] < 0) {
					numIsolates [(Math.abs(dates[ind_i][3])-min)*2]++;
					associationMap [i] [(Math.abs(dates[ind_i][3])-min)*2]++;
					dates [ind_i] [4] = (Math.abs(dates[ind_i][3])-min)*2;
				}
				else {
					numIsolates [(Math.abs(dates[ind_i][3])-min)*2 +1]++;
					associationMap [i] [(Math.abs(dates[ind_i][3])-min)*2 +1]++;
					dates [ind_i] [4] = (Math.abs(dates[ind_i][3])-min)*2 +1;
				}
			}
			else {
				numIsolates [dates[ind_i][3]-min]++;
				associationMap [i][dates[ind_i][3]-min]++;
				dates [ind_i] [4] = dates[ind_i][3]-min;
			}
			
			if (duplicates [i][1].length() > 0) {
				tmpSplit = duplicates [i][1].split(",");
				for (int j = 0; j < tmpSplit.length; j++) {
					ind_j = indexOf (dataTable,tmpSplit [j],0);
					if (time.charAt(0) == 'b') {
						if (dates [ind_j][3] < 0) {
							numIsolates [(Math.abs(dates[ind_j][3])-min)*2]++;
							associationMap [i] [(Math.abs(dates[ind_j][3])-min)*2]++;
							dates [ind_j] [4] = (Math.abs(dates[ind_j][3])-min)*2;
						}
						else {
							numIsolates [(Math.abs(dates[ind_j][3])-min)*2 +1]++;
							associationMap [i] [(Math.abs(dates[ind_j][3])-min)*2 +1]++;
							dates [ind_j] [4] = (Math.abs(dates[ind_j][3])-min)*2 +1;
						}
					}
					else {
						numIsolates [dates[ind_j][3]-min]++;
						associationMap [i][dates[ind_j][3]-min]++;
						dates [ind_j] [4] = dates[ind_j][3]-min;
					}
				}
			}
		}
		
		try {
			BufferedWriter	bw = new BufferedWriter (new FileWriter (new File (duplicatesFile + ".numIsolates")));
			String	newLine = System.getProperty ("line.separator");
			
			for (int i = 0; i < numIsolates.length; i++) bw.write (numIsolates [i] + " ");
			bw.write(newLine);
			bw.close ();
			
			bw = new BufferedWriter (new FileWriter (new File (duplicatesFile + ".associationMap")));
			for (int i = 0; i < associationMap.length; i++) {
				for (int j = 0; j < associationMap [i].length; j++) bw.write (associationMap [i][j] + "\t");
				bw.write(newLine);
			}
			bw.close();
			
			bw = new BufferedWriter (new FileWriter (new File (duplicatesFile + ".season")));
			for (int i = 0; i < dates.length; i++) bw.write(dataTable[i][0] + "\t" + dates[i][4] + newLine);
			bw.close();
		}
		catch (IOException e) {
			System.out.println ("caught the following exception: " + e);
			System.exit (1);
		}
	}
	
	// buildDataTable	input,output
	// readDataTable	fileName
	// checkDataTables	fileA,fileB
	// mergeDataTables	fileNames [] (last is output)
	// saveMapping		input,output
	// removeIdenticals	file,sortindex,index
	// writeSortedData	file,sortindex
	// writeFastaFromDataTab	input,output
	
	public static void main (String args[]) {
		if (args[0].equals("-b")) buildDataTable (args[1],args[2],(args.length > 3 ? Integer.parseInt(args[3]) : -1));
		else if (args[0].equals("-c")) checkDataTables (args[1],args[2]);
		else if (args[0].equals("-m")) {
			String	tmpArray[] = new String [args.length-1];
			System.arraycopy(args,1,tmpArray,0,args.length-1);
			mergeDataTables (tmpArray);
		}
		else if (args[0].equals("-s")) saveMapping (args[1],args[2],(args.length > 3 ? args[3] : null));
		else if (args[0].equals("-sm")) saveMapping_differentTime (args[1],args[2],args[3]);
		else if (args[0].equals("-r")) removeIdenticals (args[1],Integer.parseInt(args[2]),Integer.parseInt(args[3]),(args.length > 4 ? Boolean.parseBoolean (args[4]) : false),(args.length > 5 ? args [5] : null));
		else if (args[0].equals("-snp")) reduceSNPdata (args[1],Integer.parseInt(args[2]));
		else if (args[0].equals("-e")) enterAlignment (args[1],args[2],(args.length > 3 ? args[3] : null));
		else if (args[0].equals("-sort")) writeSortedData (args[1],Integer.parseInt(args[2]));
		else if (args[0].equals("-w")) writeFastaFromDataTab (args[1],args[2],(args.length > 3 ? Integer.parseInt(args[3]) : 0));
		else if (args[0].equals("-rare")) rarefactionTable (args[1],args[2],args[3]);
		else System.out.println("Unknown arguments!");
	}
}