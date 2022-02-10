import java.io.*;
import java.util.*;
import java.util.concurrent.*;

public class Map implements Callable<ArrayList<Object>> {
    File file;
    int x = 0;
    int offset, nr_bytes;
    int nrBytesRead = 0;
    String separators = ";:/?~\\.,><`[]{}()!@#$%^&-_+'=*\"| \t\r\n\0";

    ArrayList<String> maxLengthWord = new ArrayList<>();
    Hashtable<Integer, Integer> wordDictionary = new Hashtable<>();

    public Map (File file, int offset, int nr_bytes) {
        this.file = file;
        this.offset = offset;
        this.nr_bytes = nr_bytes;
    }

    public ArrayList<Object> call() {
        try {
            StringBuilder buffer = new StringBuilder();
            RandomAccessFile raFile = new RandomAccessFile(file, "r");
            raFile.seek(offset);
            byte[] b = new byte[10000];

            if(offset != 0) {
                raFile.seek(offset - 1);
                int prev = raFile.read();
                int curr = raFile.read();
                nrBytesRead++;
                if(separators.indexOf(prev) == -1 && separators.indexOf(curr) == -1) {
                    while(separators.indexOf(curr) == -1 && curr != -1 && nrBytesRead < nr_bytes) {
                        curr = raFile.read();
                        nrBytesRead++;
                    }
                }
                else {
                    buffer.append((char) curr);
                }
            }

            raFile.readFully(b, offset + nrBytesRead, nr_bytes - nrBytesRead);
            String str = new String(b);
            if(str.charAt(nr_bytes- nrBytesRead - 1) == ' ') {
                buffer.append(str.trim());
                buffer.append(" ");
            } else
                buffer.append(str.trim());

            int next = raFile.read();
            if(separators.indexOf(buffer.charAt(buffer.length() - 1)) == -1 && next != -1) {
                while (separators.indexOf(next) == -1 && next != -1) {
                    buffer.append((char) next);
                    next = raFile.read();
                }
            }
            String words = buffer.toString();
            String[] splitWords = words.split("[-;:/?~.,><`{}()!@#$%^&_+'=*\"| \t\r\n\0]");

            for(String w : splitWords) {
                if(maxLengthWord.isEmpty())
                    maxLengthWord.add(w);
                else {
                    if (maxLengthWord.get(0).length() == w.length()) {
                        maxLengthWord.add(w);
                    } else if (maxLengthWord.get(0).length() < w.length()) {
                        maxLengthWord.clear();
                        maxLengthWord.add(w);
                    }
                }

                if(w.length() != 0) {
                    if (!wordDictionary.containsKey(w.length()))
                        wordDictionary.put(w.length(), 1);
                    else {
                        x = wordDictionary.get(w.length());
                        x++;
                        wordDictionary.replace(w.length(), x);
                    }
                }
            }
            ArrayList<Object> results = new ArrayList<>();
            results.add(file.getName());
            results.add(wordDictionary);
            results.add(maxLengthWord);
            return results;

        } catch (IOException e) {
            System.out.println("File Not Found. Check if the file path is correct then try again.");
            e.printStackTrace();
        }
        return null;
    }
}
