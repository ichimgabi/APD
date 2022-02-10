Tema 2 - APD - Ichim Dan-Gabriel 331CC

Tema2.java
- Am rezolvat tema folosind 2 executorService, cu un awaitTermination intre ele pentru a le separa
- Primul for ia fiecare document si contorizeaza pozitia fiecarui fragment (offset) fata de inceput 
pentru a-l trimite taskului Map
- Ultimul task Map primeste lastFragmentSize ca dimensiune, nu dimensiunea data in fisierul de intrare
- Dupa tpe.shutdown si awaitTermination, incepe assignarea taskurilor Reduce
- Al doilea for se ocupa de agregarea rezultatelor din Map, pe care le pune intr-un 
ArrayList (partialResults) ce contine dictionarul si lista cu cele mai lungi cuvinte al fiecarui 
fisier testat
- Rezultatele Reduce nu erau in ordine descrescatoare, asa ca m-am folosit de un snippet de cod 
pe care l-am gasit pe internet si pe care l-am modificat pentru a face sortarea (linkul de 
reference este comentat la linia 101)

Map.java
- Ma folosesc de RandomAccessFile pentru a citi un fisier incepand cu offset-ul (asta este prima 
clasa pe care am gasit-o in care puteam folosi seek, asa ca am folosit-o)
- Citesc prima litera a fragmentului curent si ultima litera a celui anterior pentru a vedea 
daca m-am oprit la jumatatea unui cuvant, pentru a-l ignora. Dupa ce trec de el, citesc pana 
la offset+lungimea fragmentului.
- Verific daca m-am oprit la jumatatea unui cuvant, iar daca am facut-o adauf litera cu litera 
cuvantul
	
- Pun fragmentul citit intr-un String si incep sa analizez fiecare cuvant, dupa care ii pun 
detaliile intr-un HashTable si il adaug, daca este necesar in lista cuvintelor de lungime maxima

Reduce.java
- Iau rezultatele partiale trimise de catre Main si le combin intr-un singur ArrayList care are 
cuvinte de lungime maxima si un singur HashMap
- Trec prin HashMap-ul final si calculez rangul dupa formula data