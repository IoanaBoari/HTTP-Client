// Boari Ioana-Ruxandra 322CD

            Tema 4.Client Web. Comunicatie cu Rest API.

La rularea scriptului checker.py trec toate testele pentru toate script-urile oferite, inclusiv ALL.       

    Pentru implementarea acestei teme am pornit de la laboratorul 9.
Am pastrat fisierele buffer.c, buffer.h, helpers.c, helpers.h, Makefile, requests.c si requests.h.
Am folosit functiile compute_get_request pentru a face request-uri de tip GET la server 
si functia compute_post_request pentru a face request-uri de tip POST. Aceste functii le-am pastrat din laborator.
Pentru a realiza comanda "delete_book" am avut nevoie sa creez functia compute_delete_request, care
este foarte asemanatoare cu cea de compute_get_request, doar ca aceasta executa request-uri de tip DELETE.
Acum la sfarsitul temei imi dau seama ca o solutie mai buna ar fi fost sa fac o singura functie pentru get si delete,
deoarece difera doar tipul request-urilor.

    Am implementat clientul HTTP pentru toate comenzile din cerinta: register, login, enter_library,
get_books, get_book, add_book, delete_book, logout, exit. Implementarea propriu-zisa a clientului
se afla in fisierul "client.c".
Am folsit biblioteca "parson", deoarece aceasta a fost cea recomandata in cerinta,
pentru a trimite mesaje catre server si pentru a interpreta raspunsul primit.

    Comenzile se citesc de la tastatura intr-o bucla while(1) din care se iese doar la introducerea
comenzii "exit", atunci cand programul se inchide.

register: pentru a executa comanda "register" am creat functia register_user.
Aceasta comanda executa o cerere de tip POST catre server continand username si password
introduse de la tastatura. Inainte de a se trimite, datele introduse trebuie validate,
adica verific daca username-ul sau password-ul contin spatii. In acest caz se intoarce
un mesaj de eroare. Am ales sa folosesc bibilioteca "parson" pentru a parsa in format json.
Astfel, pentru a trimite username si password catre server ma folosesc de functiile din aceasta
bibilioteca pentru a crea un fisier json ce este mai usor de transmis.
Daca raspunsul de la server este unul de succes, adica s-a putut crea contul, afisez
un mesaj de succes, iar altfel inseamna ca exista deja un cont cu username-ul respectiv
si afisez un mesaj corespunzator.

login: pentru a executa comanda "login" am creat functia login_user.
Aceasta comanda este asemanatoarea cu cea de register, diferenta fiind la interperetarea raspunsului de la server.
Daca raspunsul este unul de succes se afiseaza un mesaj corespunzator si se retine cookie-ul de sesiune ce asigura
logarea utilizatorului. Exista 2 cazuri de eroare ce apar atunci cand username-ul folosit nu a fost inregistrat si
atunci cand parola este gresita.

enter_library; pentru a executa comanda "enter_library" am creat functia enter_library.
Pentru a avea acces la biblioteca trebuie sa existe un utilizator logat, astfel ca 
mai intai se verifica existenta cookie-ului. Daca acesta nu exista se genereaza un mesaj de eroare,
altfel se executa un request de tip GET si se retine token-ul generat ce asigura accesul la biblioteca.

get_books: pentru a executa comanda "get_books" am creat functia get_books.
Mai intai se verifica daca exista cookie si token pentru a asigura accesul la biblioteca.
Am ales sa le verific pe amandoua pentru a genera mesaje de eroare mai explicite.
Ar fi fost suficient sa verific doar token pentru ca fara el automat userul nu are 
acces la biblioteca. Am implementat functia extract_books_json pentru a extrage
informatiile despre carti din raspunsul serverului.

get_book: pentru a executa comanda "get_book" am creat functia get_book.
Comanda este asemanatoarea cu cea de get_books. Difera faptul ca se cere input
de la utilizator pentru id-ul cartii. Daca id-ul nu este nu numar sau nu exista 
nicio carte cu acel id se genereaza mesaje de eroare. Pentru a interpreta raspunsul 
de la server am implementat functia extract_book_json si 
astfel afisez informatiile despre cartea dorita.

add_book: pentru a executa comanda "add_book" am creat functia add_book.
Pentru aceasta comanda se fac aceleasi verificari de cookie si token ca la 
get_books si get_book. Utilizatorul trebuie sa ofere informatii despre cartea
ce vrea sa o adauge, precum: title, author, genre, publisher, page_count.
Apoi trebuie vaidata corectitudinea acestor date. Daca pentru unul dintre
campuri nu s-a introdus nimic inseamna ca datele sunt incomplete si cartea
nu poate fi adaugata. In plus, daca pentru page_count nu s-a introdus un numar
se genereaza un alt mesaj de eroare. Daca toate datele sunt corecte, se creaza un obiect json
si se executa cererea de tip POST.

delete_book: pentru a executa comanda "delete_book" am creat functia delete_book.
Comanda este asemanatoarea cu cea de get_book deoarece este nevoie de id-ul de la 
utilizator exact ca la comanda get_book. Diferenta este ca de data aceasta trebuie sa se
execute o cerere de tip DELETE catre server.

logout: pentru a executa comanda "logout" am creat functia logout.
Se verifica daca exista cookie, adica un utilizator logat.
Daca nu exista se genereaza un mesaj de eroare, altfel se delogheaza utilizatorului
si se sterge cookie-ul si token-ul folosit de acesta.

exit: pentru comanda "exit" se executa "break" din bucla while(1) si astfel se inchide programul
si se elibereaza resursele folosite.

In final tin sa mentionez ca tema a fost una interesanta cu un nivel de dificultate potrivit
ce mi s-a parut mult mai abordabila decat celelalte teme.