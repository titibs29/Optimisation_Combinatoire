// main.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <string>
#include <algorithm>


struct Entree {
    int nbCouverture = 0;
    int nbEmplacement = 0;
    std::vector<int> nbImpressionParCouverture;
    float coutImpression = 0.0;
    float coutFabrication = 0.0;
};

struct Solution {
    std::vector<int> agencement;
    std::vector<int> nbImpression;
    float coutTotal = 0.0;
};

void init(Solution* current, int nbPlaquesAndEmplacements, int nbPlaques);
void ImpressionParPlaque(Solution* current, std::vector<float>* inputData, int const& nbCouverture, float* nbEmplacement);
void GenerationPlaques(Solution* current, int nbPlaquesAGenerer, float* nbCouverture, float* nbEmplacement);
void CalculCout(Solution* current, float* nbEmplacement, float* coutFeuille, float* coutPlaque);
bool CheckValiditePlaque(Solution* current, int const& nbCouverture);
void ecriture(int nbEmplacement, Solution* best, int nbDataset);
bool lecture(Entree * entree, int nbDataset);
void GenerationImpression(Solution* current, float* nbEmplacement, int* maxImpression);
bool CheckValiditeImpression(std::vector<int> const& nbImpressionParCouverture, int const& nbCouverture, Solution* current, float* nbEmplacement);





int main(int argc, char* argv[])
{
    try{
        //declarations
        std::vector<float> inputData;
        int nbdataset;
        bool fichierLu;
        Entree entree;
        Solution best;
        Solution current;



        // recuperation des donnees dans le fichier en entree
        std::cout << "Quel dataset a tester ? : ";
        std::cin >> nbdataset;
        fichierLu = lecture(&entree, nbdataset);
        if (!fichierLu) {
            // si une erreur survient a l'ouverture
            throw 1;
        }



        /*PARTIE TEST*/
        int nbPlaques = 2;

        init(&current, nbPlaques * inputData[1], nbPlaques);

        do {
            GenerationPlaques(&current, nbPlaques, &inputData[0], &inputData[1]);

        } while (!CheckValiditePlaque(&current, inputData[0]));

        ImpressionParPlaque(&current, &inputData, inputData[0], &inputData[1]);
        CalculCout(&current, &inputData[1], &inputData[inputData.size() - 2], &inputData[inputData.size() - 1]);


        // ecriture du fichier de sortie
        ecriture(entree.nbEmplacement, &current, &nbdataset);

    }
    // gestion des erreurs
    catch (int numErreur) {
        if (numErreur == 1) {
            std::cout << "erreur a la lecture du fichier en entree" << std::endl;
        }

    }
}


/// <summary>
/// permet de valider si le nombre d'impressions nous permet d'avoir assez de couvertures
/// </summary>
/// <param name="nbImpressionParCouverture"></param>
/// <param name="nbCouverture"></param>
/// <param name="current"></param>
/// <param name="nbEmplacement"></param>
/// <returns></returns>
bool CheckValiditeImpression(std::vector<int> const&nbImpressionParCouverture,int const& nbCouverture,Solution* current, float* nbEmplacement) {
    
    std::vector<int> totalImpression(nbCouverture);
    for(int valeur : current->agencement)
    {
        totalImpression[valeur] += valeur * current->nbImpression[valeur / (int)*nbEmplacement];
    }
    for(int i=0; i < nbImpressionParCouverture.size();i++)
    {
        if (totalImpression[i] < nbImpressionParCouverture[i])return false;
    }
    return true;
}


/// <summary>
/// permet de generer aleatoirement le nombre d'impressions
/// </summary>
/// <param name="current"></param>
/// <param name="nbEmplacement"></param>
/// <param name="maxImpression"></param>
void GenerationImpression(Solution* current, float* nbEmplacement,int* maxImpression) {
    int nbPlaques = current->agencement.size() % ((int)*nbEmplacement - 1);
    for (int i = 0; i < nbPlaques; i++) {
        current->nbImpression[i] = rand() % *maxImpression;
    }
}



/// <summary>
/// lis le fichier en entree
/// </summary>
/// <param name="entree"> structure en entree de l'algo</param>
/// <param name="nbDataset"> numero de dataset</param>
bool lecture(Entree* entree, int nbDataset) {

    std::ifstream fichier("Dataset-Dev/I" + std::to_string(nbDataset) + ".in");  //Ouverture d'un fichier en lecture

    if (fichier.is_open())
    {
        // Tout est prêt pour la lecture.
        std::string donnee;
        std::vector<std::string> inputData;

        // on prends tout
        while (getline(fichier, donnee)) {
            inputData.push_back(donnee);
        }
        // on en à plus besoin
        fichier.close();
        

        // on redistribue proprement
        entree->nbCouverture = stoi(inputData[0]);
        entree->nbEmplacement = stoi(inputData[1]);

        // on boucle pour toutes les impressions
        for (int ligne = 0; ligne < inputData.size() - 3; ligne++) {
            entree->nbImpressionParCouverture.push_back(stoi(inputData[ligne+2]));
        }

        // on subdivise la dernière ligne
        int taillePremier = inputData.back().find(" ");
        entree->coutImpression = stof(inputData.back().substr(0, taillePremier));
        entree->coutFabrication = stof(inputData.back().substr(taillePremier+2, inputData.back().find(" ")));

        return 1;
    }
    else
    {
        return 0;
    }
}


/// <summary>
/// permet d'ecrire le fichier de sortie
/// </summary>
/// <param name="nbEmplacement">nombre d'emplacement sur une plaque</param>
/// <param name="best">solution a ecrire</param>
/// <param name="nbDataset">numero du dataset en entree</param>
void ecriture(int nbEmplacement, Solution* best, int nbDataset) {
    std::ofstream fichier("output/O" + std::to_string(nbDataset) + ".out");
    if (fichier)
    {
        int nbPlaques = best->agencement.size() % (nbEmplacement-1);
        fichier << nbPlaques << std::endl;
        for(int i = 0;i < nbPlaques;i++) fichier << best->nbImpression[i] << std::endl;
        for (int i = 0; i < best->agencement.size(); i++) {
            fichier << best->agencement[i];
            if (i % nbEmplacement == (nbEmplacement - 1)) fichier << std::endl;
            else fichier << ",";
        }
        fichier << best->coutTotal;
    }
    else
    {
        std::cout << "ERREUR: Impossible d'ouvrir le fichier." << std::endl;
    }
   
}


/// <summary>
/// verifie si les plaques generees sont valides
/// </summary>
/// <param name="current"></param>
/// <param name="nbCouverture"></param>
/// <returns></returns>
bool CheckValiditePlaque(Solution* current, int const& nbCouverture) {

    std::vector<int> checkApparationNombre(nbCouverture,0);
    for (int valeur : current->agencement)
    {
        checkApparationNombre[valeur] = 1;
    }
    if ((std::count(checkApparationNombre.begin(), checkApparationNombre.end(), 0))) return false;
    return true;  
}


/// <summary>
/// calcule le cout de production des couvertures
/// </summary>
/// <param name="current"></param>
/// <param name="nbEmplacement"></param>
/// <param name="coutFeuille"></param>
/// <param name="coutPlaque"></param>
void CalculCout(Solution* current, float*nbEmplacement,float*coutFeuille,float*coutPlaque) {
    
    int nbImpression = 0;
    int nbPlaques = current->agencement.size() % ((int)*nbEmplacement-1);
    for (int i = 0; i < nbPlaques; i++) {
        nbImpression += current->nbImpression[i];
    }
    current->coutTotal =  (nbImpression * *coutFeuille) + (nbPlaques * *coutPlaque) ;
}


/// <summary>
/// genere la structure des plaques
/// </summary>
/// <param name="current"></param>
/// <param name="nbPlaquesAGenerer"></param>
/// <param name="nbCouverture"></param>
/// <param name="nbEmplacement"></param>
void GenerationPlaques(Solution* current,int nbPlaquesAGenerer, float* nbCouverture, float* nbEmplacement) {
    for (int i = 0; i < nbPlaquesAGenerer; i++) {
        for (int j = 0; j < *nbEmplacement; j++) {
            current->agencement[(i * (*nbEmplacement))+j] = rand() % (int)*nbCouverture;
        }
    }
}


/// <summary>
/// calcule le nombre d'impression de chaque plaques (non-optimise)
/// </summary>
/// <param name="current"></param>
/// <param name="inputData"></param>
/// <param name="nbCouverture"></param>
/// <param name="nbEmplacement"></param>
void ImpressionParPlaque(Solution* current, std::vector<float>* inputData, int const& nbCouverture, float* nbEmplacement) {
    std::vector<int> bufferIteration(nbCouverture);
    int impressionPlaque;
    int nbPlaques = current->agencement.size() % ((int)*nbEmplacement - 1);
    for (int i = 0; i < current->agencement.size(); i++) {
        bufferIteration[current->agencement[i]] += 1;
    }
    for (int i = 0; i < bufferIteration.size(); i++) {
        bufferIteration[i] = (int)inputData->at(2 + i) / bufferIteration[i];               //greedy
    }
    for (int i = 0; i < nbPlaques; i++) {
        for (int j = 0; j < *nbEmplacement; j++) {
            if (current->nbImpression[i] < bufferIteration[current->agencement[(i * (*nbEmplacement)) + j]]) {
                current->nbImpression[i] = bufferIteration[current->agencement[(i * (*nbEmplacement)) + j]];
            }
        }
    }
    
}

/// <summary>
/// setup initial 
/// </summary>
/// <param name="current"></param>
/// <param name="nbPlaquesAndEmplacements"></param>
/// <param name="nbPlaques"></param>
void init(Solution* current, int nbPlaquesAndEmplacements, int nbPlaques) {
    for (int i = 0; i < nbPlaquesAndEmplacements; i++) {

        current->agencement.push_back(0);
    }
    for (int i = 0; i < nbPlaques; i++) {

        current->nbImpression.push_back(0);
    }
}

