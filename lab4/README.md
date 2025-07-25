# Laboratorium 4

## Tworzenie procesów. Środowisko procesu, sterowanie procesami

### Zadanie 1

Napisz program, który przyjmuje jeden argument: `argv[1]`. Program ma utworzyć `argv[1]` procesów potomnych. Każdy proces potomny ma wypisać na standardowym wyjściu w jednym wierszu dwa identyfikatory: identyfikator procesu macierzystego i swój własny. Na końcu standardowego wyjścia proces macierzysty ma wypisać `argv[1]`.  
**Wskazówka:** aby program na pewno wypisywał `argv[1]` jako ostatni wiersz standardowego wyjścia, należy użyć funkcji systemowej `wait()`.

---

### Zadanie 2

Napisz program, który przyjmuje jeden argument: `argv[1]` — ścieżkę katalogu. Program powinien wypisać na standardowym wyjściu swoją nazwę, korzystając z funkcji `printf()`. Zadeklaruj zmienną globalną `global`, a następnie zmienną lokalną `local`. W zależności od zwróconej wartości przez `fork()` dokonaj obsługi błędu, wykonaj proces rodzica / proces potomny.

**W procesie potomnym:**
- wyświetl komunikat „child process”,
- dokonaj inkrementacji zmiennych `global` i `local`,
- wyświetl komunikat „child pid = %d, parent pid = %d”
- wyświetl komunikat „child's local = %d, child's global = %d”
- wykonaj program `/bin/ls` z argumentem `argv[1]`, korzystając z funkcji `execl()`, zwracając przy tym jej kod błędu.

**W procesie rodzica:**
- wyświetl komunikat „parent process”
- wyświetl komunikat “parent pid = %d, child pid = %d”
- wyświetl komunikat “Child exit code: %d”
- wyświetl komunikat “Parent's local = %d, parent's global = %d”
- zwróć stosowny kod błędu.