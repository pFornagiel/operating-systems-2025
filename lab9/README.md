# Laboratorium 9

## Zadanie 1

Napisz program, który liczy numerycznie wartość całki oznaczonej z funkcji `4/(x2+1)` w przedziale od 0 do 1 metodą prostokątów (z definicji całki oznaczonej Riemanna). Program będzie podobny do zadania 1 w zestawie 6, jednak do przyspieszenia obliczeń zamiast procesów użyjemy wątki. Pierwszy parametr programu to szerokość każdego prostokąta, określająca dokładność obliczeń. Obliczenia należy rozdzielić na **k wątków**, tak by każdy z nich liczył inny fragment ustalonego wyżej przedziału. Każdy z wątków powinien wynik swojej części obliczeń umieścić w odpowiednim miejscu przeznaczonej do tego tablicy wyników. Wątek główny powinien oczekiwać na wyniki uzyskane od wszystkich wątków.

Ponieważ dopiero na następnym laboratorium zapoznamy się z metodami synchronizacji wątków, można do tego celu z pominięciem tych mechanizmów użyć tablicy, w której każdy z wątków liczących po umieszczeniu wyników swoich obliczeń w tablicy wyników umieszcza np wartość 1. Nazwijmy tę tablicę **tablicą gotowości**. Wątek główny sprawdza zatem czy wszystkie wartości w tablicy gotowości wynoszą 1 i jeśli tak to dodaje wyniki z tablicy wyników i wyświetla wynik na standardowym wyjściu. Zamiast powyższego można również przed dodaniem wyników zwróconych przez wątki czekać na zakończenie wszystkich wątków.

Dokładności obliczeń należy dobrać w ten sposób by obliczenia trwały co najmniej kilka sekund.