// Grasp.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <string>

using namespace std;

struct Solution {
    vector<int> agencement;
    vector<int> nbImpression;
    int coutTotal;
};
/*
struct InputData {
    int nbCouverture;
    int nbEmplacement;
    vector<int> nbImpressionParCouverture;
    int coutImpression;
    int coutFabrication;
};

struct OutputData
{
    int nbPlaque;
    vector<int> nbImpressionParPlaques;
    vector<int> tabPlaques;
    int coutTotal;
};
*/

void lecture(vector<float>* input, int* nbDataset) {

    ifstream fichier("Dataset-Dev/I" + to_string(*nbDataset) + ".in");  //Ouverture d'un fichier en lecture

    if (fichier)
    {
        //Tout est prêt pour la lecture.
        string donnee;

        while(fichier >> donnee){
            input->push_back(stof(donnee));
        }
        
    }
    else
    {
        cout << "ERREUR: Impossible d'ouvrir le fichier en lecture." << endl;
    }
}

void ecriture(float* nbEmplacement, Solution* best, int* nbDataset) {
    ofstream fichier("output/O" + to_string(*nbDataset) + ".out");
    if (fichier)
    {
        int nbPlaques = best->agencement.size() % ((int)*nbEmplacement-1);
        fichier << nbPlaques << endl;
        for(int i = 0;i < nbPlaques;i++) fichier << best->nbImpression[i] << endl;
        for (int i = 0; i < best->agencement.size(); i++) {
            fichier << best->agencement[i];
            if (i % (int)*nbEmplacement == ((int)*nbEmplacement - 1)) fichier << endl;
            else fichier << ",";
        }
        fichier << best->coutTotal;
    }
    else
    {
        cout << "ERREUR: Impossible d'ouvrir le fichier." << endl;
    }
   
}

int main()
{

    vector<float> inputData;
    Solution best;
    int nbdataset;
    cout << "Quel dataset a tester ? : ";
    cin >> nbdataset;
    lecture(&inputData, &nbdataset);
    /*
    best.agencement.push_back(0);
    best.agencement.push_back(1);
    best.agencement.push_back(2);
    best.agencement.push_back(2);
    best.agencement.push_back(0);
    best.agencement.push_back(1);
    best.agencement.push_back(2);
    best.agencement.push_back(2);
    best.nbImpression.push_back(5000);
    best.nbImpression.push_back(5500);
    best.coutTotal = 4555;
    */
    ecriture(&inputData[1], &best, &nbdataset);
}

