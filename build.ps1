$lib = "src\Population.c","src\Random.c","src\Survival.c","src\simulation.c","src\mt19937ar.c"
gcc -O2 -Isrc -o bin\lapins src\lapin.c $lib -lm
gcc -O2 -Isrc -o bin\tests  tests\main.c $lib -lm
