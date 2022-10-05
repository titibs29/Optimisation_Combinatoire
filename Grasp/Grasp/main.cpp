// main.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>   
#include <chrono>

#define RUNTIME 6000000    // temps de la boucle (1 min = 60000000)

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
void calculCout(Solution* current,unsigned int *nbEmplacement, float *coutFeuille, float *coutPlaque);
void impressionParPlaque(Solution* current,std::vector<unsigned int>* nbImpressions, unsigned int *nbCouverture, unsigned int *nbEmplacement);
bool checkValiditePlaque(Solution* current,unsigned int *nbCouverture);
void init(Solution* current, unsigned int *nbEmplacements, unsigned int *nbPlaques);
void generationPlaques(Solution* current, unsigned int *nbCouverture, unsigned int *nbEmplacement);





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
        // si une erreur survient a l'ouverture
        if (!fichierLu) throw 1;

        // attribution de la valeur la plus importante possible a best
        best.coutTotal = INT_MAX;

        /*-----METAHEURISTIQUE-----*/
        start = std::chrono::system_clock::now();

        int nb_min_plaque = entree.nbCouverture / entree.nbEmplacement;
        if (entree.nbCouverture % entree.nbEmplacement != 0) {
            nb_min_plaque += 1;
        }
        /* FIRST PASS */

        do {
            /* LOOP */

            // monkey search
            current.nbPlaques = rand() % (entree.nbCouverture - nb_min_plaque) + nb_min_plaque + 1;      // TODO - changer le 1 pour le nombre minimum de plaques possible pour avoir une copie de chaque couverture
            init(&current, &entree.nbEmplacement, &current.nbPlaques);

            // cette boucle genere un set et finit quand elle a un set valide
            do {

                // monkey search
                generationPlaques(&current, &entree.nbCouverture, &entree.nbEmplacement);

               plaquesGenerees += 1;    // stat

            } while ( !checkValiditePlaque(&current, &entree.nbCouverture));
            iterations += 1;    // stat


            // defini un nombre d'impression et calcule le cout de cette configuration
            impressionParPlaque(&current,&entree.nbImpressionParCouverture, &entree.nbCouverture, &entree.nbEmplacement);
            calculCout(&current, &entree.nbEmplacement, &entree.coutImpression, &entree.coutFabrication);

            // si meilleur, remplace le meilleur actuel
            if (current.coutTotal < best.coutTotal) {

                newBest += 1;   // stat
                best.nbPlaques = current.nbPlaques;
                best.agencement.assign(current.agencement.begin(), current.agencement.end());
                best.nbImpression.assign(current.nbImpression.begin(), current.nbImpression.end());
                best.coutTotal = current.coutTotal;
            }
        // se finit si le temps depuis start est egal ou superieur a 60 secondes
        } while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start).count() < RUNTIME);
        
        int nbImpressions = 0;
        for (unsigned int i = 0; i < best.nbPlaques; i++) {
            nbImpressions += best.nbImpression[i];
        }

        std::cout << "fini !" << std::endl;
        std::cout << "nombres d'iterations dans la boucle: " << iterations << std::endl;
        std::cout << "nombre de plaques differentes generees: " << plaquesGenerees << std::endl;
        std::cout << "nombre de nouveau best realise: " << newBest << std::endl;



        /*-----FIN-----*/
        // ecriture du fichier de sortie
        fichierEcrit = ecriture(&best, &entree.nbEmplacement, &nbdataset);
        if (!fichierEcrit) throw 99;
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
        entree->coutFabrication = stof(inputData.back().substr(taillePremier+1, inputData.back().find(" ")));

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
/// <param name="solution"> solution a ecrire</param>
/// <param name="nbEmplacement"> nombre d'emplacement sur une plaque</param>
/// <param name="nbDataset"> numero du dataset en entree</param>
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
void calculCout(Solution* current,unsigned int *nbEmplacement,float *coutImpression,float *coutFabrication) {
    
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
/// <param name="nbImpressions"></param>
/// <param name="nbCouverture"></param>
/// <param name="nbEmplacement"></param>
void impressionParPlaque(Solution* current, std::vector<unsigned int>* nbImpressions, unsigned int *nbCouverture,unsigned int *nbEmplacement) {

    // creation du buffer de valeurs
    std::vector<unsigned int> bufferIteration(*nbCouverture,0);

    // lecture du nombre d'iteration de chaque couverture dans les agancements
    for (int i = 0; i < current->agencement.size(); i++) {
        bufferIteration[current->agencement[i]] += 1;
    }

    // division du nombre d'impression par le nombre d'iteration
    for (int i = 0; i < bufferIteration.size(); i++) {
        bufferIteration[i] = ceil(nbImpressions->at(i) / bufferIteration[i]);
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
bool checkValiditePlaque(Solution* current,unsigned int *nbCouverture) {

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
/// <param name="nbCouverture"></param>
/// <param name="nbEmplacement"></param>
void generationPlaques(Solution* current, unsigned int *nbCouverture, unsigned int *nbEmplacement) {
    for (int i = 0; i < current->nbPlaques; i++) {
        for (int j = 0; j < *nbEmplacement; j++) {
            current->agencement[(i * (*nbEmplacement))+j] = rand() % *nbCouverture;
        }
    }
}



/// <summary>
/// remplissage des tableaux de la solution de valeurs nulles 
/// </summary>
/// <param name="current"></param>
/// <param name="nbEmplacements"></param>
/// <param name="nbPlaques"></param>
void init(Solution* current, unsigned int *nbEmplacements, unsigned int *nbPlaques) {

    // remplit la table agancement de valeurs nulles
    current->agencement.assign(current->nbPlaques * (*nbEmplacements), 0);

    // remplit la table nbImpression de valeurs nulles
    current->nbImpression.assign(*nbPlaques, 0);

    current->nbPlaques = *nbPlaques;
}
