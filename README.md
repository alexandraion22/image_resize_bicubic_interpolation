# Temele 1 + 2 SM - Image Resize with Bicubic Interpolation

## Realizarea temelor

- Nume : `Ion Alexandra Ana-Maria`
- Grupa : `341C1`
- Timp de lucru :` ~40 de ore`
- Recomand citirea README-ului in formatul grafic corespunzator :)



## Algoritm Original - Image Resize with Bicubic Interpolation

Am utilizat ca sursa codul din [acest repository](https://github.com/fredyedev/Bicubic-Interpolation), dar l-am modificat destul de mult:

- In cadrul codului sursa, factorul putea fi doar de marire, fiind un numar intreg, dar am modificat algoritmul astfel incat sa functioneze si cu numerele reale pentru upsizing/downsizing
- Prin rularea codului sursa original se obtineau imagini cu niste artefacte sub forma de linii negre la finalul imaginii. Am remediat aceasta problema prin calculul corect al pixelilor din marginile pozelor



## Moduri de paralelizare

### Pthreads
In cadrul paralelizarii cu Pthreads am utilizat o structura ce retine toate argumentele necesare unui thread. 

Paralelizarea a fost realizata
prin "impartirea" imaginii la toate thread-urile, fiecare thread lucrand doar pe H(height)/N(threadNum) linii din imaginea originala. Imaginea este impartita
prin calcularea indicilor start si end (care reprezinta start_height si end_height).

Imaginea in sine nu este descompusa pe randuri in structuri diferite, ci toate thread-urile au acces la toata imaginea originala si pot modifica imaginea finala, dar vor lucra doar pe liniile asignate lor.

### OpenMP
Paralelizarea cu OpenMP presupune doar paralelizarea for-urilor din cadrul functiei ImageInterpolate utilizand #pragma omp parallel for. Am utilizat si collapse(2)
pentru for-urile imbricate pentru care se putea utiliza. Scheduling-ul este lasat pe auto, dar am testat si am avut performante similare si pentu static, guided, dynamic.(Voi prezenta mai pe larg detalii despre performanta in tema 3).

### MPI
In paralelizarea utilizand MPI modul de rezolvare implica impartirea imaginii la N procese (fiecare proces va avea cate o structura diferita). 

Procesul cu rangul MASTER se va ocupa de impartirea imaginii initiale si trimiterea bucatilor din imagine catre celelalte procese (asigurandu-se si 
de cazul limita in care procesul cu rang-ul = size-1 trebuie sa ia mai multe linii din poza initiala daca inaltimea imaginii nu se imparte perfect la N). 

Procesarea fiecarei bucati din imagine este realizata identic cu pasii din rezolvarea seriala, doar ca se realizeaza in paralel de catre N procese. La finalul procesarii, acestea trimit rezultatele inapoi la procesul MASTER (care proceseaza si el partea sa de imagine) si combina rezultatele primite in imaginea rezultat si o salveaza.

### MPI + Pthreads
Aceasta paralelziare este efectiv varianta hibrida a rezolvarilor cu MPI si Pthreads separate. Am pornit de la rezolvarea ce utilizeaza doar MPI, cu aditia faptului ca fiecare proces va vrea M thread-uri, la fel ca in rezolvarea de Pthreads. 

Imaginea este impartita mai intai la N procese, iar fiecare
proces porneste M thread-uri care lucreaza simultan la cate o parte mica din bucata de imagine trimisa procesului. Astfel fiecare thread va lucra practic pe H/(M*N) linii din imagine.

La final bucatile procesate din imagine originala sunt trimise procesului MASTER pentru combinarea rezultatelor in imaginea finala.

### MPI + OpenMP
Singura modificare fata de varianta ce foloseste doar MPI este paralelizarea for-ului din functia ImageInterpolate (exact cum a fost facut in implementarea doar cu OpenMP). In rest modul de rezolvare este identic.



## Rularea Temei
Pentru rularea temei este suficient sa fie rulat scriptul "run.sh" prezent in fiecare folder respectiv fiecarei metode de paralelizare astfel:

### MPI
	./run.sh <factor> <numar_procese>

### Pthread, OpenMP
	./run.sh <factor> <numar_threaduri>

### MPI + Pthread, MPI + OpenMP
	./run.sh <factor> <numar_procese> <numar_threaduri_pe_proces>

Scriptul ruleaza programul corespunzator cu argumentele date si foloseste by default imaginea "Kittens.png" din folderul "input".
Daca se doreste testarea cu alta imagine, doar poate fi adauga alta imagine in folderul "input" si schimbat denumirea imaginii in script.

***IMPORTANT***

Rezultatele rularilor pot fi vizualizate in folderele /output/<metoda_paralelizare>.

Spre exemplu daca am rulat scriptul din folderul mpi,
rezultatul programului utilizand MPI se va gasi in /output/mpi/ cu numele "Factor_Kittens.png", in functie de factorul dat ca input.



## Dificultati Intampinate

Cred ca cea mai grea parte a fost paralelizarea cu MPI. Initial am crezut ca pot face send doar la imaginea 1D citita, dar dat fiind modul in care este reprezentata,
cele 3 canale de culoare nu sunt continue in reprezentarea 1D, astfel a trebuit sa trimit fiecare canal de culoare in parte.

O alta problema consta in faptul ca paralelizarea nu poate fi realizata utilizand Scatter/Gather, dat fiind faptul ca exista cazuri in care inaltimea pozei nu se imparte exact la numarul de procese, asa
ca am optat pentru utilziarea Send/Receive.

## Referinte

1. Documentatie
   - [Wikipedia - Bicubic Interpolation](https://en.m.wikipedia.org/wiki/Bicubic_interpolation)
2. Resurse
   - [Github (fredyedev/Bicubic-Interpolation) - Sursa Cod Rezolvare Seriala](https://github.com/fredyedev/Bicubic-Interpolation)
   - [Resizing Images With Bicubic Interpolation](https://blog.demofox.org/2015/08/15/resizing-images-with-bicubic-interpolation/)
   - [Github (omarsalazars/ImageResizing) - Comparatii diferite metode de resize image](https://github.com/omarsalazars/ImageResizing)