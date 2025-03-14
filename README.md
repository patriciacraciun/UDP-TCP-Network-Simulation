# UDP-TCP-Network-Simulation

Protocoale de comunicatii - Tema 4, C

  In acest program, am implementat un client pentru un sistem de gestionare a
bibliotecii, care permite utilizatorilor sa efectueze diverse operatiuni cum ar
fi inregistrarea, autentificarea, accesarea bibliotecii, vizualizarea cartilor,
adaugarea de carti noi si stergerea cartilor existente. Programul
interactioneaza cu un server remote prin cereri HTTP pentru a realiza aceste
operatiuni. Am folosit resurse din laboratorul 9, pe care l-am folosit drept schelet de cod. !!!

    
- inregistrarea unui utilizator nou : implica citirea username-ului si a parolei de la utilizator, verificarea daca acestea contin spatii si trimiterea unei cereri POST catre server cu aceste informatii in format JSON. Daca inregistrarea este reusita, serverul raspunde cu un mesaj de succes.
- autentificarea : userii isi introduc username-ul si parola, iar programul trimite aceste credentiale intr-o cerere POST catre server. Daca autentificarea este reusita, serverul returneaza un cookie de sesiune, care este stocat si utilizat pentru operatiunile viitoare.
- accesarea bibliotecii : user-ul trebuie sa fie autentificat; apoi, trimit o cerere GET cu cookie-ul pentru a obtine un token de acces (JWT), care va fi stocat pentru operatiunile viitoare.
- vizualizarea cartilor disponibile : realizata prin trimiterea unei cereri GET catre server cu cookie-ul si token-ul. Serverul raspunde cu o lista de carti, care este afisata utilizatorului. (este o actiune privilegiata pentru cei care au token)
- adaugarea unei carti : necesita introducerea detaliilor cartii. Dupa validarea acestor date, trimitem o cerere POST cu aceste informatii in format JSON, impreuna cu cookie-ul si token-ul. Daca serverul confirma adaugarea cu succes a cartii, utilizatorul primeste un mesaj de succes.
- stergerea unei carti : specificarea ID-ului cartii si trimiterea unei cereri DELETE catre server. Daca stergerea este reusita, utilizatorul primeste un mesaj de confirmare.
- delogarea user-ului : se face prin trimiterea unei cereri GET cu cookie-ul de sesiune catre server. Daca delogarea este reusita, utilizatorul primeste un mesaj de confirmare si cookie-ul de sesiune este invalidat.
- comanda "exit" : inchide conexiunea cu serverul si elibereaza memoria alocata pentru cookie si token, daca acestea exista.

  Pentru parsarea si gestionarea datelor in format JSON, am ales sa
folosesc biblioteca parson. Aceasta biblioteca este simpla si usor de
utilizat, oferind functionalitati complete pentru manipularea datelor
JSON. Parson permite crearea, modificarea si serializarea obiectelor
JSON intr-un mod eficient, facilitand astfel interactiunea cu serverul
prin intermediul cererilor HTTP.
