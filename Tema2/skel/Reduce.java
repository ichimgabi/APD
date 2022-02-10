import java.io.*;
import java.util.*;
import java.util.Map;
import java.util.concurrent.*;

public class Reduce implements Callable<HashMap<String, String>> {
    File file;
    ArrayList<Object> partialResults;

    public Reduce (File file, ArrayList<Object> partialResults) {
        this.file = file;
        this.partialResults = partialResults;
    }

    public float fib(float nr) {
        float fibNumber = 2;
        float prev = 1, aux;

        if(nr == 1) return 1;
        if(nr == 2) return 2;
        for(int i = 3; i < nr; i++) {
            aux = fibNumber;
            fibNumber += prev;
            prev = aux;
        }

        return fibNumber;
    }

    public HashMap<String, String> call() {
        @SuppressWarnings("unchecked")
        ArrayList<Hashtable<Integer, Integer>> dictionaryArray =
                (ArrayList<Hashtable<Integer, Integer>>) partialResults.get(0);
        @SuppressWarnings("unchecked")
        ArrayList<String> maxLengthWord = (ArrayList<String>) partialResults.get(1);

        //Combinarea listelor de cuvinte maximale pentru a pastra cele mai lungi cuvinte
        int maxLength = maxLengthWord.get(0).length();
        ArrayList<String> auxMaxLength = new ArrayList<>();
        auxMaxLength.add(maxLengthWord.get(0));

        for(int i = 1; i < maxLengthWord.size(); i++) {
            if(maxLengthWord.get(i).length() > maxLength) {
                auxMaxLength.clear();
                auxMaxLength.add(maxLengthWord.get(i));
                maxLength = maxLengthWord.get(i).length();
            }
            else if(maxLengthWord.get(i).length() == maxLength)
                auxMaxLength.add(maxLengthWord.get(i));
        }

        //Combinarea dictionarelor partiale in unul singur
        Hashtable<Integer, Integer> wordDictionary = new Hashtable<>(dictionaryArray.get(0));

        for(int i = 1; i < dictionaryArray.size(); i++) {
            dictionaryArray.get(i).forEach(
                    (key, value) -> wordDictionary.merge(key, value, Integer::sum)
            );
        }

        //Calcularea rangului
        double rang = 0;
        double nrCuv = 0;

        for (Map.Entry<Integer, Integer> entry : wordDictionary.entrySet()) {
            rang += fib(1 + entry.getKey()) * entry.getValue();
            nrCuv += entry.getValue();
        }
        rang /= nrCuv;

        HashMap<String, String> res = new HashMap<>();
        res.put(file.getName() +  "," + String.format("%.2f", rang) + "," + maxLength + "," +
                auxMaxLength.size(), String.format("%.2f", rang));
        return res;
    }
}
