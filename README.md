# Semestrálne zadanie z predmetu OS

* Implementujte dynamickú knižnicu riadiacu prácu s RAM výpočtového systému.
* Vypracované zadanie musí byť **kompilovateľné, spustiteľné a testovateľné** na vami pridelenom stroji v stave, v akom ste ho dostali na začiatku semestra.
* Za zmenu inštalovaného kompilátora alebo iných nástrojov potrebných na zostavenie zadania zodpovedajú pridelení študenti.

## Mapa projektu

* súbor *Makefile*: skript nástroja `make` na zostavenie a testovanie knižnice. Tento súbor **neupravujte**.
* adresár *src*:

  * *metadata*: súbor s údajmi o autorovi zadania – **nutné vyplniť**

    * **AIS_ID** – váš identifikátor v AIS (číslo)
    * **AIS_LOGIN** – váš prihlasovací identifikátor do AIS (napr. *xpriezvisko*)
    * **FULL_NAME** – vaše meno a priezvisko (prípadne ďalšie mená)
  * *.h súbory*: obsahujú popis API volaní, ktoré budete implementovať. Tieto súbory **neupravujte**.
  * vaše *.h* a *.c* súbory.
* adresár *build*: vznikne po spustení príkazu `make`.

## Postup

* Odporúčame vytvoriť samostatný zdrojový *.c* súbor pre každý existujúci hlavičkový *.h* súbor.
* V prípade, že na vypracovanie zadania potrebujete ďalšie verejné funkcie, vytvorte si pre ne nový *.h* súbor. Existujúce *.h* súbory **neupravujte**.
* Odporúčame najprv implementovať prazdne telá chýbajúcich funkcii.
* Pracujte v malych prírastkoch aby ste vedeli identifikovat prípadné chyby skoro.
* Akýkoľvek stav je nutné ukladať do pamäte RAM, ktorá bola inicializovaná funkciou `init_ram()` resp. zmenená volaním `init_taskMgr()`.
* Ošetrite aj hraničné prípady. Napr. v prípade, že sa stav úloh a ram nezmestí do pridelenej pamäte musí knižnica vrátiť chybu.
* Lokálne premenné vytvárané vo funkciách nemusia byť uložené v ram.
* Nesmiete použiť žiadnu alokačnú funkciu ako je malloc, alloc...
* Neodkladajte vypracovanie zadania na poslednú chvýľu pretože:
  * V prípade otázok môžete ku koncu semestra dostavať odpovede oneskorene.
  * V prípade nepredvýdaných problémov nebudete stíhať vypracovať zadanie.

## Build a test

* Príkaz `make` vykonaný v adresári projektu skompiluje, zlinkuje a vytvorí knižnicu na základe vašich zdrojových súborov a spustí testy.
* `make help` zobrazí ďalšie možnosti.

## Odovzdanie

* Váš kód udržujte štruktúrovaný a čitateľný čo zjednoduší hodnotenie a prípadné konzultácie.
  * Neschopnosť vysvetliť vaše riešenie môže viesť k udeleniu 0 bodov.
* Príkaz `make zip` vytvorí v adresári *build* archív z adresára *src* vo formáte `<AIS_LOGIN>_<AIS_ID>.zip`.
* Vytvorený archív si stiahnite do svojho počítača a odovzdajte do pripraveného miesta odovzdania v AIS.
  * Archív musí byť v uvedenom formáte. Nesplnenie formátu môže viesť k udeleniu 0 bodov.
* Na stiahnutie archívu použite príkaz `scp` alebo jeho ekvivalent.
* Úspešné vykonanie všetkých testov **negarantuje** udelenie 100% hodnotenia zadania.

## Riešenie problémov

* V prípade problémov sa obráťte e-mailom na prednášajúceho predmetu.
<br><br><br>

# Semester Assignment for the OS Course

* Implement a dynamic library that manages the operation of the computational system’s RAM.
* The completed assignment must be **compilable, executable, and testable** on the machine assigned to you, in the same state as it was at the beginning of the semester.
* Any changes to the installed compiler or other tools required to build the assignment are the responsibility of the assigned students.

## Project Structure

* *Makefile* – a script for the `make` tool used to build and test the library. **Do not modify** this file.
* *src* directory:

  * *metadata* – a file containing information about the author of the assignment – **must be filled in**

    * **AIS_ID** – your identifier in AIS (number)
    * **AIS_LOGIN** – your AIS login (e.g., *xlastname*)
    * **FULL_NAME** – your full name (first name, last name, and other names if applicable)
  * *.h files* – contain API definitions that you will implement. **Do not modify** these files.
  * your own *.h* and *.c* source files.
* *build* directory – will be created after running the `make` command.

## Procedure

* It is recommended to create a separate source *.c* file for each existing header *.h* file.
* If you need additional public functions to complete the assignment, create a new *.h* file for them. **Do not modify** the existing *.h* files.
* It is recommended to first implement empty bodies for the missing functions.
* Work in small increments so that you can identify potential errors early.
* Any state information must be stored in the RAM initialized by the `init_ram()` function or modified by the `init_taskMgr()` call.
* Handle edge cases as well. For example, if the state of tasks and RAM cannot fit into the allocated memory, the software must return an error.
* Local variables created within functions do not need to be stored in RAM.
* You must not use any allocation functions such as `malloc`, `alloc`, etc.
* Do not postpone working on the assignment until the last moment because:

  * Toward the end of the semester, responses to your questions may be delayed.
  * In case of unexpected problems, you may not have enough time to complete the assignment.

## Build and Test

* Running the `make` command in the project directory will compile, link, and build the library from your source files and execute the tests.
* `make help` will display additional options.

## Submission

* Keep your code structured and readable, as this will simplify grading and potential consultations.

  * Inability to explain your solution may result in receiving 0 points.
* The `make zip` command will create an archive in the *build* directory from the *src* directory in the format `<AIS_LOGIN>_<AIS_ID>.zip`.
* Download the generated archive to your computer and submit it to the designated submission area in AIS.
    * Archive has to be in defined format. Wrong format may result in receiving 0 points.
* Use the `scp` command or its equivalent to download the archive.
* Successful execution of all tests does **not guarantee** a 100% score for the assignment.

## Troubleshooting

* In case of any issues, contact the course lecturer via email.
