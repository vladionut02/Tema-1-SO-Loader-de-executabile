
# Tema 1 - Loader de executabile

- Nume: Varzaru Vlad
- Grupa: 325 CD

Pentru inceput, am creat o structura cu ajutorul careia tinem evidenta paginilor mapate. Asadar, avem vectorul care stocheaza paginile mapate
in timpul procesului si variabila "nr", care creste la fiecare mapare.
In legatura cu interceptarea page fault-urilor, le-am tratat in funcite de segmentul din care fac parte, asa cum este specificat in enuntul temei.
Am folosit un flag, pentru a verifica daca pagina este deja mapata sau nu. Daca valoarea flag-ului este 1, inseamna ca pagina este deja mapata
si se ruleaza handler-ul default, altfel mapam pagina in memorie si copiem datele din segmentul din fisier.

Resurse utilizate :
Laboratoare (Old but gold) :
*Creare makefile și biblioteci ( https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-01] )
*Semnale ( https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-04 )
*Gestiunea memoriei ( https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-05 )
*Memoria virtuală ( https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-06 )