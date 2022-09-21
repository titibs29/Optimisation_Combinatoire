// Grasp.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <list>

using namespace std;

int main()
{
    std::cout << "Hello World!\n";



}

struct InputData {
    int nbCouverture;
    int nbEmplacement;
    int nbImpressionParCouverture[125];
    int coutImpression;
    int coutFabrication;
};

struct OutputData
{
    int nbPlaque;
    int nbImpressionParPlaques[10];
    int tabPlaques[25][10];
    int coutTotal;
};