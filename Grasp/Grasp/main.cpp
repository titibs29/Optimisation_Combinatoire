// main.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>

#define RUNTIME 60000000    // temps de la boucle (1 min = 60000000)

struct Entree {
    unsigned int nbCouverture = 0;
    unsigned int nbEmplacement = 0;
    std::vector<unsigned int> nbImpressionParCouverture;
    float coutImpression = 0.0;
    float coutFabrication = 0.0;
};

struct Solution {
    unsigned int nbPlaques = 0;
    std::vector<unsigned int> nbImpression;
    std::vector<unsigned int> agencement;
    float coutTotal = 0.0;
};

bool lecture(Entree * entree,unsigned int* nbDataset);
bool ecriture(Solution* solution, unsigned int *nbEmplacement, unsigned int *nbDataset);
void CalculCout(Solution* current,unsigned int *nbEmplacement, float *coutFeuille, float *coutPlaque);
void ImpressionParPlaque(Solution* current,std::vector<unsigned int> nbImpressions, unsigned int *nbCouverture, unsigned int *nbEmplacement);
bool CheckValiditePlaque(Solution* current,unsigned int *nbCouverture);
void init(Solution* current, unsigned int nbPlaquesAndEmplacements, unsigned int *nbPlaques);
void GenerationPlaques(Solution* current, unsigned int *nbCouverture, unsigned int *nbEmplacement);





int main(int argc, char* argv[])
{
    try{
        /*-----SETUP-----*/
        //declarations
        unsigned int nbdataset;
        unsigned long int iterations = 1, plaquesGenerees = 0, newBest = 0;
        bool fichierLu, fichierEcrit;
        Entree entree;
        Solution current, best;

        // gestion du temps
        std::chrono::time_point<std::chrono::system_clock> start;
        unsigned long int microseconds = 0;


        // recuperation des donnees dans le fichier en entree
        std::cout << "Quel dataset a tester ? : ";
        std::cin >> nbdataset;
        fichierLu = lecture(&entree, &nbdataset);
        if (!fichierLu)
            // si une erreur survient a l'ouverture
            throw 1;

        // attribution de la valeur la plus importante possible a best
        best.coutTotal = INT_MAX;

        /*-----METAHEURISTIQUE-----*/
        start = std::chrono::system_clock::now();
        /* FIRST PASS */

        do {
            /* LOOP */

            // monkey  search
            current.nbPlaques = rand() % entree.nbCouverture + 1;
            init(&current, current.nbPlaques * entree.nbEmplacement, &current.nbPlaques);

            // cette boucle genere un set et finit quand elle a un set valide
            do {
               GenerationPlaques(&current, &entree.nbCouverture, &entree.nbEmplacement);

               plaquesGenerees += 1;    // stat

            } while ( !CheckValiditePlaque(&current, &entree.nbCouverture));
            iterations += 1;    // stat


            // defini un nombre d'impression et calcule le cout de cette configuration
            ImpressionParPlaque(&current,entree.nbImpressionParCouverture, &entree.nbCouverture, &entree.nbEmplacement);
            CalculCout(&current, &entree.nbEmplacement, &entree.coutImpression, &entree.coutFabrication);

            // si meilleur, remplace le meilleur actuel
            if (current.coutTotal < best.coutTotal) {

                newBest += 1;   // stat
                best.nbPlaques = current.nbPlaques;
                best.agencement.clear();
                best.agencement.assign(current.agencement.begin(), current.agencement.end());
                best.nbImpression.clear();
                best.nbImpression.assign(current.nbImpression.begin(), current.nbImpression.end());
                best.coutTotal = current.coutTotal;
            }

        // se finit si le temps depuis start est egal ou superieur a 60 secondes
        } while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start).count() < RUNTIME);
        std::cout << "fini !" << std::endl;
        std::cout << "nombres d'iterations dans la boucle: " << iterations << std::endl;
        std::cout << "nombre de plaques differentes generees: " << plaquesGenerees << std::endl;
        std::cout << "nombre de nouveau best realise: " << newBest << std::endl;



        


        /*-----FIN-----*/
        // ecriture du fichier de sortie
        fichierEcrit = ecriture(&best, &entree.nbEmplacement, &nbdataset);
        if (!fichierEcrit)
            throw 99;
        system(("notepad output/O" + std::to_string(nbdataset) + ".out").c_str()); // ouvre le fichier de sortie


    }
    // gestion des erreurs
    catch (unsigned int e) {
        if (e == 1) {
            std::cout << "erreur a la lecture du fichier en entree" << std::endl;
        }
        if (e == 99) {
            std::cout << "erreur lors de l'ecriture du fichier de sortie" << std::endl;
        }

    }
}



/// <summary>
/// lis le fichier en entree
/// </summary>
/// <param name="entree"> structure en entree de l'algo</param>
/// <param name="nbDataset"> numero de dataset</param>
bool lecture(Entree* entree, unsigned int* nbDataset) {

    std::ifstream fichier("Dataset-Dev/I" + std::to_string(*nbDataset) + ".in");  //Ouverture d'un fichier en lecture

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
        for (unsigned int ligne = 0; ligne < inputData.size() - 3; ligne++) {
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
bool ecriture(Solution* solution, unsigned int* nbEmplacement, unsigned int* nbDataset) {

    std::ofstream fichier("output/O" + std::to_string(*nbDataset) + ".out");
    if (fichier.is_open())
    {
        // nombre de plaques
        fichier << solution->nbPlaques << std::endl;

        // les impressions pour chaque plaque
        for(unsigned int i = 0;i < solution->nbImpression.size(); i++)
            fichier << solution->nbImpression[i] << std::endl;

        // les agencements de chaque plaque
        for (unsigned int i = 0; i < solution->agencement.size(); i++) {
            fichier << solution->agencement[i];
            if (i % *nbEmplacement == (*nbEmplacement - 1)) fichier << std::endl;
            else fichier << ", ";
        }

        // cout total de la solution
        fichier << std::to_string(solution->coutTotal);

        fichier.close();
        return 1;
    }
    else
    {
        return 0;
    }
   
}

/// <summary>
/// calcule le cout de production des couvertures
/// </summary>
/// <param name="current"></param>
/// <param name="nbEmplacement"></param>
/// <param name="coutImpression"> cout par impression</param>
/// <param name="coutFabrication"> cout par fabrication</param>
void CalculCout(Solution* current,unsigned int *nbEmplacement,float *coutImpression,float *coutFabrication) {
    
    int nbImpressions = 0;
    for (unsigned int i = 0; i < current->nbPlaques; i++) {
        nbImpressions += current->nbImpression[i];
    }
    current->coutTotal =  (nbImpressions * *coutImpression) + (current->nbPlaques * *coutFabrication) ;
}

/// <summary>
/// calcule le nombre d'impression de chaque plaques (non-optimise)
/// </summary>
/// <param name="current"></param>
/// <param name="inputData"></param>
/// <param name="nbCouverture"></param>
/// <param name="nbEmplacement"></param>
void ImpressionParPlaque(Solution* current, std::vector<unsigned int> nbImpressions, unsigned int *nbCouverture,unsigned int *nbEmplacement) {

    // creation du buffer de valeurs
    std::vector<unsigned int> bufferIteration(*nbCouverture,0);

    // lecture du nombre d'iteration de chaque couverture dans les agancements
    for (int i = 0; i < current->agencement.size(); i++) {
        bufferIteration[current->agencement[i]] += 1;
    }

    // division du nombre d'impression par le nombre d'iteration
    for (int i = 0; i < bufferIteration.size(); i++) {
        bufferIteration[i] = ceil(nbImpressions[i] / bufferIteration[i]);
    }

    // determination du nombre de passage minimum par chaque plaque
    for (int i = 0; i < current->nbPlaques; i++) {
        for (int j = 0; j < *nbEmplacement; j++) {
            if (current->nbImpression[i] < bufferIteration[current->agencement[(i * (*nbEmplacement)) + j]]) {
                current->nbImpression[i] = bufferIteration[current->agencement[(i * (*nbEmplacement)) + j]];
            }
        }
    }
    
}

/// <summary>
/// verifie si les plaques generees sont valides
/// </summary>
/// <param name="current"></param>
/// <param name="nbCouverture"></param>
/// <returns></returns>
bool CheckValiditePlaque(Solution* current,unsigned int *nbCouverture) {

    try {

    std::vector<bool> checkApparationNombre(*nbCouverture);

    // boucle dans toutes les cases
    for (unsigned int valeur : current->agencement)
    {
        checkApparationNombre[valeur] = true;
    }

    // verifie si tout les nombres sont present
    if ((std::count(checkApparationNombre.begin(), checkApparationNombre.end(), 0))) return false;

    return true;  
    }
    catch (int e) {
        return false;
    }
}


/// <summary>
/// genere la structure des plaques aleatoirement
/// </summary>
/// <param name="current"></param>
/// <param name="nbPlaquesAGenerer"></param>
/// <param name="nbCouverture"></param>
/// <param name="nbEmplacement"></param>
void GenerationPlaques(Solution* current, unsigned int *nbCouverture, unsigned int *nbEmplacement) {

    for (int i = 0; i < current->nbPlaques; i++) {
        for (int j = 0; j < *nbEmplacement; j++) {
            current->agencement[(i * (*nbEmplacement))+j] = rand() % *nbCouverture;
        }
    }
}



/// <summary>
/// setup initial 
/// </summary>
/// <param name="current"></param>
/// <param name="nbPlaquesAndEmplacements"></param>
/// <param name="nbPlaques"></param>
void init(Solution* current, unsigned int nbPlaquesAndEmplacements, unsigned int *nbPlaques) {

    // remplit la table agancement de valeurs nulles
    current->agencement.clear();
    for (int i = 0; i < nbPlaquesAndEmplacements; i++) {

        current->agencement.push_back(0);
    }

    // remplit la table nbImpression de valeurs nulles
    current->nbImpression.clear();
    for (int i = 0; i < *nbPlaques; i++) {

        current->nbImpression.push_back(0);
    }

    current->nbPlaques = *nbPlaques;
}
