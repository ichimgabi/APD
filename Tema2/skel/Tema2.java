import java.io.*;
import java.util.*;
import java.util.concurrent.*;
import java.util.stream.Collectors;

public class Tema2 {
    public static void main(String[] args)
            throws IOException {

        if (args.length < 3) {
            System.err.println("Usage: Tema2 <workers> <in_file> <out_file>");
            return;
        }

        File input = new File(args[1]);
        Scanner sc = new Scanner(input);
        String out = args[2];
        int P = Integer.parseInt(args[0]);
        int nr_bytes, nr_doc;
        int offset;
        ArrayList<String> numeFisiere = new ArrayList<>();
        ArrayList<ArrayList<Object>> results = new ArrayList<>();

        BufferedWriter writer = new BufferedWriter(new FileWriter(out));

        ExecutorService tpe = Executors.newFixedThreadPool(P);
        ExecutorService tpe2 = Executors.newFixedThreadPool(P);

        HashMap<String, String> finalResults = new HashMap<>();

        sc.useDelimiter("\n");
        nr_bytes = Integer.parseInt(sc.next());
        nr_doc = Integer.parseInt(sc.next());

        for(int i = 0 ; i < nr_doc; i++) {
            //Am schimbat path-ul fisiereleor de intrare din test.txt (adaugat "../" in fata)
            offset = 0;
            numeFisiere.add(sc.next());
            File f = new File(numeFisiere.get(i));
            int lastFragmentSize = (int)f.length() % nr_bytes;
            while(f.length() > offset) {
                Future<ArrayList<Object>> future = tpe.submit(new Map(f, offset, nr_bytes));
                try {
                    results.add(future.get());
                } catch (InterruptedException ie) {
                    //System.out.println("da");
                } catch (ExecutionException ee) {
                    //System.out.println("da2");
                }
                if(offset + 2L * nr_bytes < f.length())
                    offset += nr_bytes;
                else {
                    offset += nr_bytes;
                    Future<ArrayList<Object>> future2 = tpe.submit(new Map(f, offset, lastFragmentSize));
                    try {
                        results.add(future2.get());
                    } catch (InterruptedException ie) {
                        //System.out.println("da3");
                    } catch (ExecutionException ee) {
                        //System.out.println("da4");
                    }
                    break;
                }
            }

        }

        try {
            tpe.shutdown();
            tpe.awaitTermination(2, TimeUnit.SECONDS);
        } catch (InterruptedException ex) {
            System.out.println("Eroare awaitTermination tpe");
        }

        for(int i = 0; i < nr_doc; i++) {
            ArrayList<Object> partialResults = new ArrayList<>();
            ArrayList<Hashtable<Integer, Integer>> partialWordDictionary = new ArrayList<>();
            ArrayList<String> partialMaxLengthWord = new ArrayList<>();
            String resStr = numeFisiere.get(i);
            File f = new File(resStr);
            for(int j = 0; j < results.size(); j++) {
                if(results.get(j).get(0).equals(f.getName())) {
                    partialWordDictionary.add((Hashtable<Integer, Integer>) results.get(j).get(1));
                    partialMaxLengthWord.addAll((ArrayList<String>) results.get(j).get(2));
                }
            }
            partialResults.add(partialWordDictionary);
            partialResults.add(partialMaxLengthWord);
            Future<HashMap<String, String>> future = tpe2.submit(new Reduce(f, partialResults));

            try {
                finalResults.putAll(future.get());
            } catch (InterruptedException ie) {
                //System.out.println("da3");
            } catch (ExecutionException ee) {
                //System.out.println("da4");
            }
        }

        //Sorteaza rezultatele finale pentru a le pune in fisierul de output in ordine descrescatoare
        //Luat de pe https://stackabuse.com/how-to-sort-a-hashmap-by-value-in-java/
        HashMap<String, String> sortedMap = finalResults.entrySet().stream()
                .sorted(Comparator.comparingDouble(e -> -Double.parseDouble(e.getValue())))
                .collect(Collectors.toMap(
                        HashMap.Entry::getKey,
                        HashMap.Entry::getValue,
                        (a, b) -> { throw new AssertionError(); },
                        LinkedHashMap::new
                ));

        //sortedMap.entrySet().forEach(System.out::println);

        sortedMap.forEach((key, value) -> {
            try {
                writer.append(key).append("\n");
            } catch (IOException e) {
                e.printStackTrace();
            }
        });

        try {
            tpe2.shutdown();
            tpe2.awaitTermination(2, TimeUnit.SECONDS);
        } catch (InterruptedException ex) {
            System.out.println("Eroare awaitTermination tpe2");
        }
        writer.close();
        sc.close();
    }
}
