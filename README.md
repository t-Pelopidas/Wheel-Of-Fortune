### Wheel Of Fortune

### Στοιχεία
* Ομάδα: Πελοπίδας Θωμάς
* AM: 2124488
* emails
    * gmail: thomaspelopidas12@gmail.com
    * Ακαδημαϊκό email: thpelopidas@uth.gr

### Compile
* Αρχεία: server.c , client.c, Makefile
* Μεταγλώττιση: gcc

Για να γίνει compile τρέξτε την ενολή:

```bash
make all
```

Εναλλακτικά, αν αποτύχει η παραπάνω μέθοδος, εκτελέστε την εντολή:
```bash
gcc server.c -o server -Wall -Wextra -Werror -pedantic -g
gcc client.c -o client -Wall -Wextra -Werror -pedantic -g
```

### Εκτέλεση

Για την εκτέλεση χρειάζονται 4 τερματικά, 1 για τον server και 3 για τους clients.

1) Terminal 1: εκκίνηση server
```bash
./server

```
2) Terminal 2: εκκίνηση client 1
```bash
./client 127.0.0.1 4001
```

3) Terminal 3: εκκίνηση client 2
```bash
./client 127.0.0.1 4001
```

4) Terminal 4: εκκίνηση client 3
```bash
./client 127.0.0.1 4001
```
