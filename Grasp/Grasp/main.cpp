// main.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <fstream>
#include <vector>  
#include <string>
#include <chrono>
#include <thread>
#include <semaphore>
#include <mutex>

// parametres
#define RUNTIME 15				// temps de la boucle (secondes)
#define NBCANDIDATES 10			// nombre de candidats /!\ MUST BE HIGHER THAN NUMBER OF THREADS
#define NBITERLOCAL 100			// nombre d'iteration locale
#define NBTHREADS 6				// nombre de threads
#define CHANCESMOINSPLAQUES 80	// en pourcent les chances d'avoir moins de plaques que le meilleur resultat dans le candidat
#define PARTAVIRER 0.5			// sur 1, la quantité du pool de candidats à remplacer

//MULTITHREADING
std::vector<std::thread> threads;

std::mutex stat_variables_mtx;
std::mutex best_mtx;


struct Entree {
	unsigned char nbCouverture = 0;		// max 125
	unsigned char nbEmplacement = 0;	// max 25
	std::vector<unsigned int> nbImpressionParCouverture;
	float coutImpression = 0.0;
	float coutFabrication = 0.0;
};

struct Solution {
	unsigned short int nbPlaques = 0;
	std::vector<unsigned int> nbImpression;
	std::vector<unsigned char> agencement; // max 125
	float coutTotal = FLT_MAX;
	bool actif = true;
	bool isUsedInThread = false;
};

// fonctions
bool lecture(Entree* entree, std::string input);
bool ecriture(Solution* solution, unsigned char* nbEmplacement, unsigned short int* nbDataset);

void calculCout(std::vector<unsigned char>* agencement,
	std::vector<unsigned int>* nbImpression, 
	unsigned short int* nbPlaques,
	unsigned char* nbEmplacement, 
	float* coutTotal, 
	float* coutImpression,
	float* coutFabrication);
bool checkValiditePlaque(std::vector<unsigned char>* agencement, unsigned char* nbCouverture);
void impressionParPlaque(std::vector<unsigned char>* agencement,
	std::vector<unsigned int>* nbImpressions,
	std::vector<unsigned int>* nbImpressionsParCouv, 
	unsigned short int* nbPlaques,
	unsigned char* nbCouverture,
	unsigned char* nbEmplacement);

void init(Solution* current, unsigned char* nbEmplacements);
void generationPlaques(Solution* current, std::vector<float> *poidsImpression, unsigned char* nbCouverture, unsigned char* nbEmplacement);
void TableauPoids(std::vector<unsigned int>* nbImpression, std::vector<float>* poidsImpression);

void SwitchAgencement(Solution* current);
void thread(Solution* current, Entree entree);
void joinAllThreads(void);


unsigned long int iterations = 0;
unsigned long int plaquesGenerees = 0;
unsigned long int newBest = 0;

Entree entree;
Solution best;
Solution listCandidats[NBCANDIDATES];



int main(int argc, char* argv[])
{
	/*-----SETUP-----*/
	//declarations
	unsigned short int nbdataset = 1;	//max 16
	unsigned short int nbMinPlaques = 0;
	unsigned short int nbMaxPlaques = USHRT_MAX;
	unsigned int candidat = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int nbImpressions = 0;
	unsigned int pireIndice = 0;
	unsigned int random = 0;
	unsigned long int nbTotCandidats = 0;
	unsigned long int microseconds = 0;     // duree
	bool err = false;
	float pireCout = 0.0;
	std::vector<float> poidsImpression;
	
	best.coutTotal = FLT_MAX;				//attribution de la valeur la plus importante possible a best

	// gestion du temps
	std::chrono::time_point<std::chrono::system_clock> start;


	try {
		//prevent non-random behavior
		if (NBTHREADS >= NBCANDIDATES) throw 123;

		// recuperation des donnees dans le fichier en entree
		std::cout << "Quel dataset a tester ? : ";
		std::cin >> nbdataset;
		
		err = lecture(&entree, "Dataset-Dev/I" + std::to_string(nbdataset) + ".in");

		// si une erreur survient a l'ouverture
		if (err) throw 1;

		if (entree.nbEmplacement == '\0') throw 2; // nbEmplacement ne peut pas etre nul, il sera denominateur d'une division

		// nombre minimum de plaque pour avoir toutes les couvertures
		nbMinPlaques = ceil(entree.nbCouverture / (float)entree.nbEmplacement);
		nbMaxPlaques = entree.nbCouverture;

		// association d'un poids pour chaque couverture
		poidsImpression.assign(entree.nbCouverture, 0);
		TableauPoids(&entree.nbImpressionParCouverture, &poidsImpression);

		std::cout << "debut du calcul, cela va prendre " << RUNTIME << " secondes" << std::endl;


		/*-----METAHEURISTIQUE-----*/


		start = std::chrono::system_clock::now();
		/*---FIRST PASS---*/


		/* PREMIER REMPLISSAGE DE LA LISTE */
		for (; candidat < NBCANDIDATES; candidat++) {
			listCandidats[candidat].nbPlaques = (rand() % nbMaxPlaques) + nbMinPlaques;
			init(&listCandidats[candidat], &entree.nbEmplacement);
			do {

				generationPlaques(&listCandidats[candidat], &poidsImpression, &entree.nbCouverture, &entree.nbEmplacement);

				plaquesGenerees++;   // stat
			} while (!checkValiditePlaque(&listCandidats[candidat].agencement, &entree.nbCouverture));
			impressionParPlaque(&listCandidats[candidat].agencement, 
				&listCandidats[candidat].nbImpression, 
				&entree.nbImpressionParCouverture, 
				&listCandidats[candidat].nbPlaques, 
				&entree.nbCouverture, 
				&entree.nbEmplacement);
			calculCout(&listCandidats[candidat].agencement, 
				&listCandidats[candidat].nbImpression, 
				&listCandidats[candidat].nbPlaques, 
				&entree.nbEmplacement,
				&listCandidats[candidat].coutTotal,
				&entree.coutImpression,
				&entree.coutFabrication);
			nbTotCandidats++;

			// si meilleur, remplace le meilleur actuel
			if (listCandidats[candidat].coutTotal < best.coutTotal) {

				newBest += 1;   // stat
				best.nbPlaques = listCandidats[candidat].nbPlaques;
				best.agencement.assign(listCandidats[candidat].agencement.begin(), listCandidats[candidat].agencement.end());
				best.nbImpression.assign(listCandidats[candidat].nbImpression.begin(), listCandidats[candidat].nbImpression.end());
				best.coutTotal = listCandidats[candidat].coutTotal;
			}
		}


		/* CHANGEMENTS LOCAUX */
		for (int i = 0; i <  NBTHREADS; i++)
		{
			int r;
			do
			{
				r = rand() % NBCANDIDATES;
			} while (listCandidats[r].isUsedInThread);

			//Reserve candidate for not being chosen in another thread
			listCandidats[r].isUsedInThread = true;

			//starting all threads
			threads.push_back(std::thread(thread, &listCandidats[r], entree));
		}
		//wait for all threads to finish executing
		joinAllThreads();
		


		/*---LOOP---*/
		do {


			/* TRI PARMI LES CANDIDATS */
			// on vire la moitie du pool de candidat, c'est du 50/50
			for (i = 0; i < NBCANDIDATES * 0.5; i++) {
				pireCout = 0.0;

				// trouve le moins bon resultat
				for (j = 0; j < NBCANDIDATES; j++) {
					if (listCandidats[j].coutTotal > pireCout && listCandidats[j].actif) {
						pireCout = listCandidats[j].coutTotal;
						pireIndice = j;
					}
				}

				// le vire
				listCandidats[pireIndice].actif = false;

			}


			/* REMPLISSAGE DE LA LISTE */
			for (i = 0; i < NBCANDIDATES; i++) {
				// si desactive
				if (!listCandidats[i].actif) {


					// le remplis de nouvelles valeurs aleatoire
					// chances de génerer une candidat avec moins de plaques
					if (best.nbPlaques > nbMinPlaques) {
						if ((rand() % 100) < CHANCESMOINSPLAQUES) {
						listCandidats[i].nbPlaques = (rand() % (best.nbPlaques - nbMinPlaques)) + nbMinPlaques;
						}
					}
					else {
						listCandidats[i].nbPlaques = (rand() % nbMaxPlaques) + nbMinPlaques;
					}
					init(&listCandidats[i], &entree.nbEmplacement);
					do {

						generationPlaques(&listCandidats[i], &poidsImpression, &entree.nbCouverture, &entree.nbEmplacement);

						plaquesGenerees++;   // stat
					} while (!checkValiditePlaque(&listCandidats[i].agencement, &entree.nbCouverture));
					impressionParPlaque(&listCandidats[i].agencement,
						&listCandidats[i].nbImpression,
						&entree.nbImpressionParCouverture,
						&listCandidats[i].nbPlaques,
						&entree.nbCouverture,
						&entree.nbEmplacement);
					calculCout(&listCandidats[i].agencement,
						&listCandidats[i].nbImpression,
						&listCandidats[i].nbPlaques,
						&entree.nbEmplacement,
						&listCandidats[i].coutTotal,
						&entree.coutImpression,
						&entree.coutFabrication);
					listCandidats[i].actif = true;
					nbTotCandidats++;


					// si meilleur, remplace le meilleur actuel
					if (listCandidats[i].coutTotal < best.coutTotal) {

						newBest += 1;   // stat
						best.nbPlaques = listCandidats[i].nbPlaques;
						best.agencement.assign(listCandidats[i].agencement.begin(), listCandidats[i].agencement.end());
						best.nbImpression.assign(listCandidats[i].nbImpression.begin(), listCandidats[i].nbImpression.end());
						best.coutTotal = listCandidats[i].coutTotal;
					}
				}

			}


			
			/* CHANGEMENTS LOCAUX */
			for (int i = 0; i < NBTHREADS; i++)
			{
				int r;
				do
				{
					r = rand() % NBCANDIDATES;
				} while (listCandidats[r].isUsedInThread);

				//Reserve candidate for not being chosen in another thread
				listCandidats[r].isUsedInThread = false;

				//starting all threads
				threads.push_back(std::thread(thread, &listCandidats[r], entree));
			}
			//wait for all threads to finish executing
			joinAllThreads();
			


			// se finit si le temps depuis start est egal ou superieur a 60 secondes
		} while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start).count() < RUNTIME * pow(10, 6));


		std::cout << "nombres d'iterations dans la boucle: " << iterations << std::endl;
		std::cout << "nombre de plaques differentes generees: " << plaquesGenerees << std::endl;
		std::cout << "nombre de nouveau best realise: " << newBest << std::endl;
		std::cout << "nombre de candidats crees: " << nbTotCandidats << std::endl;




		/*-----FIN-----*/
		// ecriture du fichier de sortie
		err = ecriture(&best, &entree.nbEmplacement, &nbdataset);
		if (err) throw 99;
		system(("notepad output/O" + std::to_string(nbdataset) + ".out").c_str()); // ouvre le fichier de sortie


	}
	// gestion des erreurs
	catch (unsigned int e) {
		if (e == 1) {
			std::cout << "erreur a la lecture du fichier en entree" << std::endl;
		}
		if (e == 2) {
			std::cout << "division par zero impossible " << std::endl;
		}
		if (e == 3) {
			std::cout << "erreur a l'ouverture du fichier en entree" << std::endl;
		}
		if (e == 98) {
			std::cout << "erreur a l'ouverture du fichier en sortie" << std::endl;
		}
		if (e == 99) {
			std::cout << "erreur lors de l'ecriture du fichier de sortie" << std::endl;
		}
		if (e == 123)
		{
			std::cout << "The number of threads needs to exeed the number of candidates" << std::endl;
		}
		else
		{
			std::cout << "erreur inconnue" << std::endl;
		}

	}
}



/// <summary>
/// calcule le cout de production des couvertures
/// </summary>
/// <param name="current"></param>
/// <param name="nbEmplacement"></param>
/// <param name="coutImpression"> cout par impression</param>
/// <param name="coutFabrication"> cout par fabrication</param>
void calculCout(std::vector<unsigned char>* agencement,
	std::vector<unsigned int>* nbImpression,
	unsigned short int* nbPlaques,
	unsigned char* nbEmplacement,
	float* coutTotal,
	float* coutImpression,
	float* coutFabrication) {

	unsigned int totImpressions = 0;


	for (unsigned int i = 0; i < *nbPlaques; i++) {
		totImpressions += nbImpression->at(i);
	}

	*coutTotal = (totImpressions * *coutImpression) + (*nbPlaques * *coutFabrication);
}

/// <summary>
/// calcule le nombre d'impression de chaque plaques (non-optimise)
/// </summary>
/// <param name="current"></param>
/// <param name="nbImpressions"></param>
/// <param name="nbCouverture"></param>
/// <param name="nbEmplacement"></param>
void impressionParPlaque(std::vector<unsigned char>* agencement,
	std::vector<unsigned int>* nbImpressions,
	std::vector<unsigned int>* nbImpressionsParCouv,
	unsigned short int* nbPlaques,
	unsigned char* nbCouverture,
	unsigned char* nbEmplacement) {

	// creation du buffer de valeurs
	std::vector<unsigned int> bufferIteration(*nbCouverture, 0);

	// lecture du nombre d'iteration de chaque couverture dans les agencements
	for (unsigned int i = 0; i < agencement->size(); i++) {
		bufferIteration[agencement->at(i)] += 1;
	}

	// division du nombre d'impression par le nombre d'iteration
	for (unsigned int i = 0; i < bufferIteration.size(); i++) {
		bufferIteration[i] = ceil(nbImpressionsParCouv->at(i) / bufferIteration[i]);
	}

	// determination du nombre de passage minimum par chaque plaque
	for (unsigned int plaque = 0; plaque < *nbPlaques; plaque++) {
		for (unsigned int emplacement = 0; emplacement < *nbEmplacement; emplacement++) 
		{
			int o = plaque * (*nbEmplacement) + emplacement;
			if (nbImpressions->at(plaque) < bufferIteration.at(agencement->at(o)))
			{
				nbImpressions->at(plaque) = bufferIteration.at(agencement->at(o));
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
bool checkValiditePlaque(std::vector<unsigned char>* agencement, unsigned char* nbCouverture) {

		std::vector<bool> checkApparationNombre(*nbCouverture);

		// boucle dans toutes les cases
		for (unsigned int valeur : *agencement)
		{
			checkApparationNombre[valeur] = true;
		}

		// verifie si tout les nombres sont present
		if ((std::count(checkApparationNombre.begin(), checkApparationNombre.end(), 0))) return false;

		return true;
}

/// <summary>
/// code permettant de generer l'agencement des plaques
/// </summary>
/// <param name="current"></param>
/// <param name="poidsImpression"></param>
/// <param name="nbCouverture"></param>
/// <param name="nbEmplacement"></param>
void generationPlaques(Solution* current, std::vector<float>* poidsImpression, unsigned char* nbCouverture, unsigned char* nbEmplacement) {

	int c1 = 0;
	int c2 = 0;
	if (*nbEmplacement == 0 || current->nbPlaques == 0) throw 2;
	float reduction = 1.0f / (float)(*nbEmplacement * current->nbPlaques);

	for (unsigned int i = 0; i < current->nbPlaques; i++) {
		for (unsigned int j = 0; j < *nbEmplacement; j++) {

			// peuplement des deux premiers
			if (poidsImpression->at(1) < poidsImpression->at(0)) {
				c1 = 0;
				c2 = 1;
			}
			else {
				c1 = 1;
				c2 = 0;
			}

			//recherche des 2 meilleurs candidats
			for (unsigned int x = 2; x < poidsImpression->size(); x++) {
				if (poidsImpression->at(c2) < poidsImpression->at(x)) {
					if (poidsImpression->at(c1) < poidsImpression->at(x)) {
						c2 = c1;
						c1 = x;
					}
					else {
						c2 = x;
					}
				}
			}
			if (rand() % 2) {
				current->agencement[(i * (*nbEmplacement)) + j] = c2;
				poidsImpression->at(c2) -= reduction;
			}
			else {
				current->agencement[(i * (*nbEmplacement)) + j] = c1;
				poidsImpression->at(c1) -= reduction;
			}
		}
	}
}

/// <summary>
/// remplissage des tableaux de la solution de valeurs nulles 
/// </summary>
/// <param name="current"></param>
/// <param name="nbEmplacements"></param>
/// <param name="nbPlaques"></param>
void init(Solution* current, unsigned char* nbEmplacements) {

	// remplit la table agencement de valeurs nulles
	current->agencement.assign(current->nbPlaques * (*nbEmplacements), 0);

	// remplit la table nbImpression de valeurs nulles
	current->nbImpression.assign(current->nbPlaques, 0);
}

/// <summary>
/// remplissage des tableaux de la solution de valeurs nulles 
/// </summary>
/// <param name="current"></param>
/// <param name="nbEmplacements"></param>
/// <param name="nbPlaques"></param>
void TableauPoids(std::vector<unsigned int>* nbImpression, std::vector<float>* poidsImpression) {

	float total = 0;
	for (int valeur : *nbImpression) {
		total += valeur;
	}
	if (total == 0) throw 2;
	for (unsigned int i = 0; i < nbImpression->size(); i++) {
		poidsImpression->at(i) += nbImpression->at(i) / total;
	}
}

void SwitchAgencement(Solution* current) {
    if (current->nbPlaques < 2) {
        int temp = 0;
        int rand1 = 0;
        int rand2 = 0;
        int taille = current->agencement.size();

        rand1 = rand() % taille;
        rand2 = rand() % taille + (((rand() % current->nbPlaques) + 1) * taille);

        temp = current->agencement.at(rand1);
        current->agencement.at(rand1) = current->agencement.at(rand2);
        current->agencement.at(rand2) = temp;
    }
    
}

//    /!\ 'Current' must be in one thread at a time
void thread(Solution* current, Entree entree)
{
	/* CHANGEMENT LOCAUX */
	//'w' is not used inside the for loop.
	for (int w = 0; w < NBITERLOCAL; w++)
	{
		// crée un nouvel agencement
		std::vector<unsigned char> agencementBis;
		std::vector<unsigned int> impressionsBis;
		float coutBis = 0.0;
		int i = 0;


		impressionsBis.assign(current->nbPlaques, 0);

		do {
			agencementBis.assign(current->agencement.begin(), current->agencement.end());

			for (i = rand() % agencementBis.size() + 1; i > 0; i--) {
				agencementBis[rand() % agencementBis.size()] = (rand() % entree.nbCouverture);
			}

		} while (!checkValiditePlaque(&agencementBis, &entree.nbCouverture));

		
		// calcule le cout de ce nouvel agencement
		//std::cout << "A" << std::endl;
		impressionParPlaque(&agencementBis, &impressionsBis, &entree.nbImpressionParCouverture, &current->nbPlaques, &entree.nbCouverture, &entree.nbEmplacement);
		//std::cout << "APRES" << std::endl;
		calculCout(&agencementBis, &impressionsBis, &current->nbPlaques, &entree.nbEmplacement, &coutBis, &entree.coutImpression, &entree.coutFabrication);
		//std::cout << "APRES2" << std::endl;

		// compare, garde le meilleur
		if (coutBis < current->coutTotal) {

			current->agencement.assign(agencementBis.begin(), agencementBis.end());
			current->nbImpression.assign(impressionsBis.begin(), impressionsBis.end());
			current->coutTotal = coutBis;
		}

		//lock mutex for stat variables
		stat_variables_mtx.lock();

		iterations++;    // stat
		plaquesGenerees++;   // stat

		//unlock mutex for stat variables
		stat_variables_mtx.unlock();

		
		//lock mutex for best
		best_mtx.lock();

		// si meilleur, remplace le meilleur actuel
		if (current->coutTotal < best.coutTotal)
		{
			newBest += 1;   // stat
			best.nbPlaques = current->nbPlaques;
			best.agencement.assign(current->agencement.begin(), current->agencement.end());
			best.nbImpression.assign(current->nbImpression.begin(), current->nbImpression.end());
			best.coutTotal = current->coutTotal;
		}

		//unlock mutex for best
		best_mtx.unlock();

		//free candidate
		current->isUsedInThread = false;
	}
}

/// <summary>
/// lis le fichier en entree
/// </summary>
/// <param name="entree"> structure en entree de l'algo</param>
/// <param name="nbDataset"> numero de dataset</param>
bool lecture(Entree* entree, std::string input) {



	std::ifstream fichier(input);  //Ouverture d'un fichier en lecture

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
			entree->nbImpressionParCouverture.push_back(stoi(inputData[ligne + 2]));
		}

		// on subdivise la dernière ligne
		int taillePremier = inputData.back().find(" ");
		entree->coutImpression = stof(inputData.back().substr(0, taillePremier));
		entree->coutFabrication = stof(inputData.back().substr(taillePremier + 1, inputData.back().find(" ")));

		return 0;
	}
	else
	{
		throw 3;
		return 1;
	}
}

/// <summary>
/// permet d'ecrire le fichier de sortie
/// </summary>
/// <param name="solution"> solution a ecrire</param>
/// <param name="nbEmplacement"> nombre d'emplacement sur une plaque</param>
/// <param name="nbDataset"> numero du dataset en entree</param>
bool ecriture(Solution* solution, unsigned char* nbEmplacement, unsigned short int* nbDataset) {

	std::ofstream fichier("output/O" + std::to_string(*nbDataset) + ".out");
	if (fichier.is_open())
	{
		// nombre de plaques
		fichier << solution->nbPlaques << std::endl;

		// les impressions pour chaque plaque
		for (unsigned int i = 0; i < solution->nbImpression.size(); i++)
			fichier << solution->nbImpression[i] << std::endl;

		// les agencements de chaque plaque
		for (unsigned int i = 0; i < solution->agencement.size(); i++) {
			fichier << (int)solution->agencement[i];
			if (i % *nbEmplacement == (*nbEmplacement - 1)) fichier << std::endl;
			else fichier << ", ";
		}

		// cout total de la solution
		fichier << std::to_string(solution->coutTotal);

		fichier.close();
		return 0;
	}
	else
	{
		throw 98;
		return 1;
	}

}

void joinAllThreads()
{
	while (!threads.empty())
	{
		threads.front().join();
		threads.erase(threads.begin());
	}
}
