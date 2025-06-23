# Laboratorium 6

## Zadanie 1

Napisz program, który liczy numerycznie wartość całki oznaczonej z funkcji `4/(x2+1)` w przedziale od 0 do 1 metodą prostokątów (z definicji całki oznaczonej Riemanna). Pierwszy parametr programu to szerokość każdego prostokąta, określająca dokładność obliczeń. Obliczenia należy rozdzielić na **k procesów potomnych**, tak by każdy z procesów liczył inny fragment ustalonego wyżej przedziału. Każdy z procesów powinien wynik swojej części obliczeń przesłać przez potok nienazwany do procesu macierzystego. Każdy proces potomny do komunikacji z procesem macierzystym powinien używać osobnego potoku. Proces macierzysty powinien oczekiwać na wyniki uzyskane od wszystkich procesów potomnych po czym powinien dodać te wyniki cząstkowe i wyświetlić wynik na standardowym wyjściu wraz z czasem wykonania oraz odpowiadającą mu wartością k.

Program powinien przeprowadzić powyższe obliczenia dla wartości `k=1,2,...,n` (gdzie n to drugi parametr wywołania programu). W ten sposób w wyniku działania programu na standardowym wyjściu wypisane zostaną wyniki obliczeń oraz czasy wykonania tych obliczeń przy wykorzystaniu jednego procesu, dwóch procesów, trzech procesów oraz kolejnych liczb wykorzystanych procesów aż do n.

Dokładności obliczeń należy dobrać w ten sposób by obliczenia trwały co najmniej kilka sekund.

---

## Zadanie 2

Napisz program, który liczy numerycznie wartość całki oznaczonej tak jak w zadaniu 1. Obliczenia powinny zostać przeprowadzone w ten sposób, że pierwszy program czyta ze standardowego wejścia przedział w jakim całka ma być policzona a następnie przesyła przez potok nazwany do programu drugiego wartości określające ten przedział. Drugi program po otrzymaniu informacji liczy całkę w otrzymanym przedziale i odsyła do programu pierwszego wynik obliczeń. Po otrzymaniu wyniku obliczeń, program pierwszy wyświetla wynik.