# Laboratorium 2

## Biblioteki w systemie Unix

Biblioteki są kluczowymi składnikami każdego systemu operacyjnego, a ich zrozumienie jest niezbędne dla skutecznego tworzenia i zarządzania aplikacjami. W ramach tego ćwiczenia przyjrzymy się trzem głównym typom bibliotek: statycznym, współdzielonym i dynamicznym.

Biblioteki statyczne są skompilowanymi plikami binarnymi, które są dołączane do programów podczas kompilacji. Są one integralną częścią aplikacji i wszystkie funkcje zawarte w bibliotece statycznej są kopiowane do pliku wykonywalnego programu. W przeciwieństwie do tego biblioteki współdzielone są ładowane do pamięci podczas uruchamiania programu i mogą być współużytkowane przez wiele aplikacji. Natomiast biblioteki dynamiczne są podobne do bibliotek współdzielonych, ale są ładowane do pamięci podczas uruchamiania programu i mogą być ładowane i usuwane dynamicznie w trakcie działania aplikacji.

Aby zademonstrować tworzenie i korzystanie z bibliotek zaimplementujemy bibliotekę pomagającą w badaniu problemu Collatza.

---

### Problem Collatza

Problem Collatza (Collatz Conjecture), znana również jako problem 3n+1, to jedno z najbardziej znanych nierozwiązanych problemów w matematyce. Zakłada, że ​​startując od dowolnej dodatniej liczby całkowitej, można ją zredukować do 1, wykonując iteracyjne kroki na podstawie następujących reguł:  
- jeśli liczba jest parzysta, podziel ją przez 2;  
- jeśli jest nieparzysta, pomnóż przez 3 i dodaj 1.  

Mimo prostoty zasady, problem pozostaje nierozwiązany dla dowolnej liczby startowej, choć jest uznawany za prawdziwy ze względu na obserwacje empiryczne dla wielu początkowych wartości.

---

## Zadanie

1. Stwórz bibliotekę w języku C wystawiającą klientom następujące dwie funkcje:
   - `int collatz_conjecture(int input)` - funkcja realizująca regułę Collatza.

     **Collatz Conjecture**  
     Funkcja ta przyjmuje jedną liczbę typu całkowitego. Jeżeli jest ona parzysta, podziel ją przez 2 i zwróć wynik. Jeżeli jest nieparzysta, pomnóż ją przez 3 i dodaj 1, a następnie zwróć wynik.

   - `int test_collatz_convergence(int input, int max_iter, int *steps)` - funkcja sprawdzająca po ilu wywołaniach collatz_conjecture zbiega się do 1. Powinna ona wywoływać regułę Collatza najpierw na liczbie wejściowej a później na wyniku otrzymanym z reguły. W celu ochrony przed zbyt długim zapętlaniem się funkcji drugi parametr stanowi górną granicę liczby iteracji. Steps jest wyściowym argumentem zawieraczącym listę wynikową. Funkcja zwraca rozmiar tablicy steps czyli liczbę wykonanych kroków.

2. Zwróć do wywołującej funkcji sekwencję liczb prowadzącą do redukcji `input` do 1. W przypadku, gdy nie było to możliwe w `max_iter`, zwróć z funkcji 0.

3. W pliku makefile utwórz dwa wpisy: do kompilacji statycznej biblioteki i do kompilacji współdzielonej.

4. Napisz klienta korzystającego z kodu biblioteki, klient powinien sprawdzać kilka liczb, wykorzystując test_collatz_convergence, tj. po ilu iteracjach wynik zbiegnie się do 1 i wypisać całą sekwencję redukcji `input` do 1, gdy dało się to osiągnąć w `max_iter`. W przeciwnym wypadku wypisz komunikat o niepowodzeniu.

5. Klient powinien korzystać z biblioteki na 3 sposoby:
   - Jako biblioteka statyczna
   - Jako biblioteka współdzielona (linkowana dynamicznie)
   - Jako biblioteka ładowana dynamicznie

6. Dla każdego z wariantów utwórz odpowiedni wpis w Makefile. Do realizacji biblioteki dynamicznej użyj definicji stałej (-D) i dyrektywy preprocesora, aby zmodyfikować sposób działania klienta.
