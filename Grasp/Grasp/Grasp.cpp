// Grasp.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <string>
#include <algorithm>


struct Solution {
    std::vector<int> agencement;
    std::vector<int> nbImpression;
    float coutTotal;
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

/*
 * INUTILISER POUR LE MOMENT
 * 
bool CheckValiditeImpression(std::vector<int> const&nbImpressionParCouverture,int const& nbCouverture,Solution* current, float* nbEmplacement) {
    
    std::vector<int> totalImpression(nbCouverture);
    for(int valeur : current->agencement)
    {
        totalImpression[valeur] += valeur * current->nbImpression[valeur / (int)*nbEmplacement];
    }
    for(int i; i<nbImpressionParCouverture.size();i++)
    {
        if (totalImpression[i] < nbImpressionParCouverture[i])return false;
    }
    return true;
}

void GenerationImpression(Solution* current, float* nbEmplacement,int* maxImpression) {
    int nbPlaques = current->agencement.size() % ((int)*nbEmplacement - 1);
    for (int i = 0; i < nbPlaques; i++) {
        current->nbImpression[i] = rand() % *maxImpression;
    }
}
*/



void lecture(std::vector<float>* input, int* nbDataset) {

    std::ifstream fichier("Dataset-Dev/I" + std::to_string(*nbDataset) + ".in");  //Ouverture d'un fichier en lecture

    if (fichier)
    {
        //Tout est prêt pour la lecture.
        std::string donnee;

        while(fichier >> donnee){
            input->push_back(stof(donnee));
        }
        
    }
    else
    {
        std::cout << "ERREUR: Impossible d'ouvrir le fichier en lecture." << std::endl;
    }
}

void ecriture(float* nbEmplacement, Solution* best, int* nbDataset) {
    std::ofstream fichier("output/O" + std::to_string(*nbDataset) + ".out");
    if (fichier)
    {
        int nbPlaques = best->agencement.size() % ((int)*nbEmplacement-1);
        fichier << nbPlaques << std::endl;
        for(int i = 0;i < nbPlaques;i++) fichier << best->nbImpression[i] << std::endl;
        for (int i = 0; i < best->agencement.size(); i++) {
            fichier << best->agencement[i];
            if (i % (int)*nbEmplacement == ((int)*nbEmplacement - 1)) fichier << std::endl;
            else fichier << ",";
        }
        fichier << best->coutTotal;
    }
    else
    {
        std::cout << "ERREUR: Impossible d'ouvrir le fichier." << std::endl;
    }
   
}

bool CheckValiditePlaque(Solution* current, int const& nbCouverture) {

    std::vector<int> checkApparationNombre(nbCouverture,0);
    for (int valeur : current->agencement)
    {
        checkApparationNombre[valeur] = 1;
    }
    if ((std::count(checkApparationNombre.begin(), checkApparationNombre.end(), 0))) return false;
    return true;  
}



void CalculCout(Solution* current, float*nbEmplacement,float*coutFeuille,float*coutPlaque) {
    
    int nbImpression = 0;
    int nbPlaques = current->agencement.size() % ((int)*nbEmplacement-1);
    for (int i = 0; i < nbPlaques; i++) {
        nbImpression += current->nbImpression[i];
    }
    current->coutTotal =  (nbImpression * *coutFeuille) + (nbPlaques * *coutPlaque) ;
    std::cout << *coutFeuille << std::endl;
}

void GenerationPlaques(Solution* current,int nbElementAGenerer, int* nbCouverture) {
    for (int i = 0; i < nbElementAGenerer; i++) {
        current->agencement[i] = rand() % *nbCouverture;
    }
}



void ImpressionParPlaque(Solution* current, std::vector<float>* inputData, int const& nbCouverture, float* nbEmplacement) {
    std::vector<int> bufferIteration(nbCouverture);
    int impressionPlaque;
    int nbPlaques = current->agencement.size() % ((int)*nbEmplacement - 1);
    for (int i = 0; i < current->agencement.size(); i++) {
        bufferIteration[current->agencement[i]] += 1;
    }
    for (int i = 0; i < bufferIteration.size(); i++) {
        bufferIteration[i] = (int)inputData[2 + i] / bufferIteration[i];               //greedy
    }
    for (int i = 0; i < nbPlaques; i++) {
        for (int j = 0; j < *nbEmplacement; j++) {
            if (current->nbImpression[nbPlaques] < bufferIteration[current->agencement[i * *nbEmplacement + j]]) {
                current->nbImpression[nbPlaques] = bufferIteration[current->agencement[i * *nbEmplacement + j]];
            }
        }
    }
    
}



int main()
{

    std::vector<float> inputData;
    Solution best;
    Solution current;
    int nbdataset;
    std::cout << "Quel dataset a tester ? : ";
    std::cin >> nbdataset;
    lecture(&inputData, &nbdataset);
    best.agencement.push_back(0);
    best.agencement.push_back(1);
    best.agencement.push_back(2);
    best.agencement.push_back(2);
    best.nbImpression.push_back(2);

    
    CalculCout(&best,&inputData[1], &inputData[inputData.size()-2], &inputData[inputData.size() - 1]);


    ecriture(&inputData[1], &best, &nbdataset);
}

