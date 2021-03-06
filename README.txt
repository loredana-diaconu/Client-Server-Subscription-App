TEMA 2 - Diaconu Maria-Loredana, 325CA

In aceasta tema am utilizat scheletele de la laboratoarele 6 si 8.

I. subscriber.cpp
   Inainte de toate, trimite ID la server printr-o conexiune TCP. In continuare,
   acesta poate primi de la tastatura comenzile de exit, subscribe si unsubscribe.
   Daca acestea sunt scrise corect, le trimite la server.
   Subscriber primeste mesajele dorite de la server si le afiseaza. Daca primeste
   "ID already in use", se deconecteaza.
II. server.cpp
    Realizeaza conexiunile cu clietii UDP (publisherii) si cei TCP(subscriberii).
    1. Daca citeste de la stdin, ma asigur ca acea comanda poate fi doar exit.
    2. Daca se afla pe socketul UDP, primeste mesaj de la publisher, pe care il
       transforma utilizand functia build_message din "utils.h", in functie de
       tipul de date trimise. Apoi, cauta prin vectorul de clienti, clientii abonati
       la topicul respectiv. Daca acestia sunt conectati, le trimite mesajul. Daca
       nu, iar abonamentul are SF = 1, pastrez mesajul in vectorul de mesaje neprimite
       ale clientului.
    3. Daca se afla pe socketul TCP, accepta conexiuni de la clienti. Aici verific
       daca ID-ul noului client exista deja. Daca da si este deconectat, il reconectez
       si trimit toate mesajele din bufferul de mesaje neprimite ale acestuia.
       Daca este deja conectat, clientul nou este inchis deoarece nu pot exista doi
       clienti cu acelasi ID.
       Altfel, creez un client nou pe care il adaug in vectorul de clienti.
    4. Altfel, primesc comenzi de la clienti. Daca primesc subscribe sau unsubscribe,
       actualizez vectorul de abonamente ale clientului. Altfel, conexiunea este inchisa.
III. utils.h
     Contine structuri si functii ajutatoare:
     Am utilizat structura Subscription pentru a retine un abonament, descris prin topic
     si SF, si structura Client, ce descrie un client prin ID, socketul utilizat, starea
     (daca este conectat sau nu), vectorul de abonamente si vectorul de mesaje specifice.
     Functiile subscribe si unsubscribe actualizeaza vectorii de abonamente ale clientilor.
     Am tratat cazul in care doreste sa se aboneze la un topic la care este deja abonat,
     schimbat valoarea SF daca este cazul.
     Functia build_message creeaza un mesaj, pornind de la informatia primita de la publisher,
     in functie de tipul datelor. Pentru STRING, pur si simplu am extras mesajul. Pentru
     INT, FLOAT si SHORT_REAL, am creeat numarul in functie de nr. de octeti pe care este re-
     prezentat, semn si numarul de cifre dupa virgula.
