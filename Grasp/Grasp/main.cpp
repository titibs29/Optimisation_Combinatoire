// main.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>

#define RUNTIME 3000000    // temps de la boucle (1 min = 60000000)

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

bool lecture(Entree * entree,unsigned int nbDataset);
bool ecriture(Solution* solution, unsigned int nbEmplacement, unsigned int nbDataset);
void CalculCout(Solution* current,unsigned int *nbEmplacement, float *coutFeuille, float *coutPlaque);
void ImpressionParPlaque(Solution* current,std::vector<unsigned int> nbImpressions, unsigned int nbCouverture, unsigned int nbEmplacement);
bool CheckValiditePlaque(Solution* current,unsigned int nbCouverture);
void init(Solution* current, unsigned int nbPlaquesAndEmplacements, unsigned int nbPlaques);
void GenerationPlaques(Solution* current, unsigned int nbPlaquesAGenerer, unsigned int nbCouverture, unsigned int nbEmplacement);
//void GenerationImpression(Solution* current, unsigned int nbEmplacement, unsigned int maxImpression);
//bool CheckValiditeImpression(Solution* current, std::vector<unsigned int> nbImpressionParCouverture,unsigned int nbCouverture, unsigned int nbEmplacement);





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
        fichierLu = lecture(&entree, nbdataset);
        if (!fichierLu)
            // si une erreur survient a l'ouverture
            throw 1;

        // remplissage de donnees nulles en sortie
        int nbPlaques = 2;
        init(&best, nbPlaques * entree.nbEmplacement, nbPlaques);
        init(&current, nbPlaques * entree.nbEmplacement, nbPlaques);
        best.coutTotal = INT_MAX;

        /*-----METAHEURISTIQUE-----*/
        start = std::chrono::system_clock::now();
        /* firstPass */

        do {
            /* LOOP */

            // cette boucle genere un set et finit quand elle a un set valide
            do {
               GenerationPlaques(&current, nbPlaques, entree.nbCouverture, entree.nbEmplacement);
               plaquesGenerees += 1;

            } while ( !CheckValiditePlaque(&current, entree.nbCouverture));
            iterations += 1;
            // defini un nombre d'impression et calcule le cout de cette configuration
            ImpressionParPlaque(&current,entree.nbImpressionParCouverture, entree.nbCouverture, entree.nbEmplacement);
            CalculCout(&current, &entree.nbEmplacement, &entree.coutImpression, &entree.coutFabrication);

            // si meilleur, remplace le meilleur actuel
            if (current.coutTotal < best.coutTotal) {
                newBest += 1;
                best.nbPlaques = current.nbPlaques;
                best.agencement.assign(current.agencement.begin(), current.agencement.end());
                best.nbImpression.assign(current.nbImpression.begin(), current.nbImpression.end());
                best.coutTotal = current.coutTotal;
            }

        // se finit si le temps depuis start est egal ou superieur a 60 secondes
        } while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start).count() < RUNTIME);
        std::cout << "temps ecoule" << std::endl;
        std::cout << "nombres d'iterations dans la boucle: " << iterations << std::endl;
        std::cout << "nombre de plaques differentes generees: " << plaquesGenerees << std::endl;
        std::cout << "nombre de nouveau best realise: " << newBest << std::endl;



        


        /*-----FIN-----*/
        // ecriture du fichier de sortie
        fichierEcrit = ecriture(&best, entree.nbEmplacement, nbdataset);
        if (!fichierEcrit)
            throw 99;


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
bool lecture(Entree* entree, unsigned int nbDataset) {

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
bool ecriture(Solution* solution, unsigned int nbEmplacement, unsigned int nbDataset) {

    std::ofstream fichier("output/O" + std::to_string(nbDataset) + ".out");
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
            if (i % nbEmplacement == (nbEmplacement - 1)) fichier << std::endl;
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
void ImpressionParPlaque(Solution* current, std::vector<unsigned int> nbImpressions, unsigned int nbCouverture,unsigned int nbEmplacement) {

    // creation du buffer de valeurs
    std::vector<unsigned int> bufferIteration(nbCouverture,0);

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
        for (int j = 0; j < nbEmplacement; j++) {
            if (current->nbImpression[i] < bufferIteration[current->agencement[(i * (nbEmplacement)) + j]]) {
                current->nbImpression[i] = bufferIteration[current->agencement[(i * (nbEmplacement)) + j]];
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
bool CheckValiditePlaque(Solution* current,unsigned int const nbCouverture) {

    try {

    std::vector<bool> checkApparationNombre(nbCouverture);

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
void GenerationPlaques(Solution* current, unsigned int nbPlaquesAGenerer, unsigned int nbCouverture, unsigned int nbEmplacement) {

    for (int i = 0; i < nbPlaquesAGenerer; i++) {
        for (int j = 0; j < nbEmplacement; j++) {
            current->agencement[(i * (nbEmplacement))+j] = rand() % nbCouverture;
        }
    }
}



/// <summary>
/// setup initial 
/// </summary>
/// <param name="current"></param>
/// <param name="nbPlaquesAndEmplacements"></param>
/// <param name="nbPlaques"></param>
void init(Solution* current, unsigned int nbPlaquesAndEmplacements, unsigned int nbPlaques) {

    // remplit la table agancement de valeurs nulles
    for (int i = 0; i < nbPlaquesAndEmplacements; i++) {

        current->agencement.push_back(0);
    }

    // remplit la table nbImpression de valeurs nulles
    for (int i = 0; i < nbPlaques; i++) {

        current->nbImpression.push_back(0);
    }
}


//
///// <summary>
///// permet de valider si le nombre d'impressions nous permet d'avoir assez de couvertures
///// </summary>
///// <param name="nbImpressionParCouverture"></param>
///// <param name="nbCouverture"></param>
///// <param name="current"></param>
///// <param name="nbEmplacement"></param>
///// <returns></returns>
//bool CheckValiditeImpression(std::vector<unsigned int> const&nbImpressionParCouverture,unsigned int const& nbCouverture,Solution* current, unsigned int nbEmplacement) {
//    
//    std::vector<unsigned int> totalImpression(nbCouverture);
//    for(int valeur : current->agencement)
//    {
//        totalImpression[valeur] += valeur * current->nbImpression[valeur / nbEmplacement];
//    }
//    for(unsigned int i=0; i < unsigned(nbImpressionParCouverture.size());i++)
//    {
//        if (totalImpression[i] < nbImpressionParCouverture[i])return false;
//    }
//    return true;
//}
//
//
///// <summary>
///// permet de generer aleatoirement le nombre d'impressions
///// </summary>
///// <param name="current"></param>
///// <param name="nbEmplacement"></param>
///// <param name="maxImpression"></param>
//void GenerationImpression(Solution* current, float* nbEmplacement,int* maxImpression) {
//    int nbPlaques = current->agencement.size() % ((int)*nbEmplacement - 1);
//    for (int i = 0; i < nbPlaques; i++) {
//        current->nbImpression[i] = rand() % *maxImpression;
//    }
//}
//