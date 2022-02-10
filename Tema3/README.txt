Tema 3 - 331CC - Ichim Dan-Gabriel

Codul este destul de messy, so bear with me a little bit.

read_neighbours() - Functie luata din laboratorul 10 care citeste datele din fisierele 
care descriu fiecare cluster (cluster0.txt etc.)

get_cluster() -Topologia o formez intr-un string pe care il afiseaza fiecare proces atunci cand 
este completa. Din cauza asta, am creat o functie care analizeaza stringul si extrage 
clusterul in care se afla procesul primit

get_cluster_number_neigh() - Analizeaza stringul de topolgie pentru a numara cate 
procese worker se afla in clusterul dat


Metoda de functionare:
	-Pentru fiecare proces coordonator (0, 1, 2) am cate un if() in care ii spun ce sa faca
	-Procesele worker se afla in ultimul for pentru ca au acelasi tip de comporatament

	Procesul 0:
		-Incep prin a construi stringul topologiei, adaugand rankul worker-ilor, si, 
	babeste, semnele de punctuatie (',', ' ', ':')
		-Primesc apoi de la ceilalti coordonatori vecinii lor, pe care ii append-uiesc
		-Cand am completat topologia, o trimit mai departe fiecarui coordonator si 
	proces worker (liniile 125 - 133)

		-Etapa de Calcul:
			Fiecare variabila are o alta variabila care ii reprezinta dimensiunea (pentru 
		send & receive)
			Variabile: 
				1) vector - Retine array-ul de baza, din numarul primit la input
				2) partial_vector - bucata din vector pe care o trimit celorlalte 
			procese. Se modifica dupa fiecare trimitere.
				3) remaining_partial_vector_size - in cazul in care vectorul nu se
			imparte egal la numarul de workeri, variabila asta tine cont de partea 
			ramasa pe dinafara
				4) current_element - contor pentru a sti la ce element din vector sunt

				Pentru fiecare worker trimit mai intai dimensiunea vectorului partial pe 
			care urmeaza sa il tirmit, dupa care vectorul in sine.
			In cazul in care am un vector mare care se imparte inegal, modific vectorul 
			partial pentru a lua inca o valoare in plus (scad apoi remaining_partial_vector_size)
				Fac la fel si pentru clustere, insa vectorul partial va fi mai mare (in functie) 
			de cati workeri are fiecare cluster).
				Primesc apoi datele prelucrate de la workeri si clustere, modificand 
			vectorul mare cu noile valori.

	Procesul 1 & 2:
		-Trimit datele legate de vecinii clusterului
		-Primesc topologia finala de la procesul 0

		-Etapa de calcul:
			Primesc vectorul partial de la procesul 0, dupa care il impart in mai multi 
		vectori partiali pe care ii trimit proceselor worker
			Primesc rezultatul final al prelucrarii de la workeri, dupa care trimit 
		rezuatatul procesului 0, impreuna cu dimensiunea lui.

	Procesele worker:
		-Primesc topologia de la fiecare coordonator si o afiseaza
		-Primesc vectorii partiali de la coordonatori, ii prelucreaza, iar apoi ii trimit 
		inapoi. In cazul clusterului 0, voi trimite si dimensiunea vectorului prelucrat.

