// main.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <fstream>
#include <vector>  
#include <string>
#include <chrono>

// parametres
#define RUNTIME 30           // temps de la boucle (secondes)
#define NBCANDIDATES 10     // nombre de candidats 
#define NBITERLOCAL 10000    // nombre d'iteration locale
#define NBTHREADS 4         // (useless)nombre de threads


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
	float coutTotal = FLT_MAX;
	bool actif = true;
};

// fonctions
bool lecture(Entree* entree, unsigned int* nbDataset);
bool ecriture(Solution* solution, unsigned int* nbEmplacement, unsigned int* nbDataset);

void calculCout(std::vector<unsigned int>* agencement, std::vector<unsigned int>* nbImpression, unsigned int* nbPlaques, float* coutTotal, unsigned int* nbEmplacement, float* coutImpression, float* coutFabrication);
bool checkValiditePlaque(std::vector<unsigned int>* agencement, unsigned int* nbCouverture);
void impressionParPlaque(std::vector<unsigned int>* agencement, unsigned int* nbPlaques, std::vector<unsigned int>* nbImpressions, std::vector<unsigned int>* nbImpressionsParCouv, unsigned int* nbCouverture, unsigned int* nbEmplacement);

void init(Solution* current, unsigned int* nbEmplacements);
void generationPlaques(Solution* current, std::vector<float> *poidsImpression, unsigned int* nbCouverture, unsigned int* nbEmplacement);
void TableauPoids(std::vector<unsigned int>* nbImpression, std::vector<float>* poidsImpression);

void SwitchAgencement(Solution* current);
void thread(Solution* current, Entree* entree);



int main(int argc, char* argv[])
{
	/*-----SETUP-----*/
	//declarations
	unsigned int nbdataset = 1;
	unsigned int nbMinPlaques = 0;
	unsigned int candidat = 0;
	unsigned int i = 0;
	unsigned int nbImpressions = 0;
	unsigned int pireIndice = 0;
	unsigned int random = 0;
	unsigned long int iterations = 0;
	unsigned long int plaquesGenerees = 0;
	unsigned long int newBest = 0;
	unsigned long int microseconds = 0;     // duree
	bool fichierLu, fichierEcrit;
	float pireCout = 0.0;
	std::vector<float> poidsImpression;
	Entree entree;
	Solution best;
	Solution listCandidats[NBCANDIDATES];

	// gestion du temps
	std::chrono::time_point<std::chrono::system_clock> start;


	try {

		// recuperation des donnees dans le fichier en entree
		std::cout << "Quel dataset a tester ? : ";
		std::cin >> nbdataset;
		fichierLu = lecture(&entree, &nbdataset);

		// si une erreur survient a l'ouverture
		if (!fichierLu) throw 1;

		// attribution de la valeur la plus importante possible a best
		//best.coutTotal = FLT_MAX;

		// nombre minimum de plaque pour avoir toutes les couvertures
		nbMinPlaques = ceil(entree.nbCouverture / (float)entree.nbEmplacement);

		// association d'un poids pour chaque couverture
		poidsImpression.assign(entree.nbCouverture, 0);
		TableauPoids(&entree.nbImpressionParCouverture, &poidsImpression);

		std::cout << "debut du calcul, cela va prendre " << RUNTIME << " secondes" << std::endl;
		/*-----METAHEURISTIQUE-----*/


		start = std::chrono::system_clock::now();
		/*---FIRST PASS---*/


		/* PREMIER REMPLISSAGE DE LA LISTE */
		for (; candidat < NBCANDIDATES; candidat++) {
			listCandidats[candidat].nbPlaques = (rand() % entree.nbCouverture) + nbMinPlaques;
			init(&listCandidats[candidat], &entree.nbEmplacement);
			do {

				generationPlaques(&listCandidats[candidat], &poidsImpression, &entree.nbCouverture, &entree.nbEmplacement);

				plaquesGenerees++;   // stat
			} while (!checkValiditePlaque(&listCandidats[candidat].agencement, &entree.nbCouverture));
			impressionParPlaque(&listCandidats[candidat].agencement, &listCandidats[candidat].nbPlaques, &listCandidats[candidat].nbImpression, &entree.nbImpressionParCouverture, &entree.nbCouverture, &entree.nbEmplacement);
			calculCout(&listCandidats[candidat].agencement, &listCandidats[candidat].nbImpression, &listCandidats[candidat].nbPlaques, &listCandidats[candidat].coutTotal, &entree.nbEmplacement, &entree.coutImpression, &entree.coutFabrication);

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
		for (i = 0; i < NBITERLOCAL; i++) {

			random = rand() % NBCANDIDATES;
			thread(&listCandidats[random], &entree);
			iterations += 1;    // stat
			plaquesGenerees++;   // stat

			// si meilleur, remplace le meilleur actuel
			if (listCandidats[random].coutTotal < best.coutTotal) {

				newBest += 1;   // stat
				best.nbPlaques = listCandidats[random].nbPlaques;
				best.agencement.assign(listCandidats[random].agencement.begin(), listCandidats[random].agencement.end());
				best.nbImpression.assign(listCandidats[random].nbImpression.begin(), listCandidats[random].nbImpression.end());
				best.coutTotal = listCandidats[random].coutTotal;
			}
		}


		/*---LOOP---*/
		do {


			/* TRI PARMI LES CANDIDATS */
			// on vire la moitie du pool de candidat, c'est du 50/50
			for (i = 0; i < NBCANDIDATES / 2; i++) {
				pireCout = 0.0;

				// trouve le moins bon resultat
				for (int j = 0; j < NBCANDIDATES; j++) {
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
					listCandidats[i].nbPlaques = (rand() % entree.nbCouverture) + nbMinPlaques;
					init(&listCandidats[i], &entree.nbEmplacement);
					do {

						generationPlaques(&listCandidats[i], &poidsImpression, &entree.nbCouverture, &entree.nbEmplacement);

						plaquesGenerees++;   // stat
					} while (!checkValiditePlaque(&listCandidats[i].agencement, &entree.nbCouverture));
					impressionParPlaque(&listCandidats[i].agencement, &listCandidats[i].nbPlaques, &listCandidats[i].nbImpression, &entree.nbImpressionParCouverture, &entree.nbCouverture, &entree.nbEmplacement);
					calculCout(&listCandidats[i].agencement, &listCandidats[i].nbImpression, &listCandidats[i].nbPlaques, &listCandidats[i].coutTotal, &entree.nbEmplacement, &entree.coutImpression, &entree.coutFabrication);

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


			/* CHANGEMENT LOCAUX */
			for (i = 0; i < NBITERLOCAL; i++) {

				random = rand() % NBCANDIDATES;
				thread(&listCandidats[random], &entree);
				iterations += 1;    // stat
				plaquesGenerees++;   // stat

				// si meilleur, remplace le meilleur actuel
				if (listCandidats[random].coutTotal < best.coutTotal) {

					newBest += 1;   // stat
					best.nbPlaques = listCandidats[random].nbPlaques;
					best.agencement.assign(listCandidats[random].agencement.begin(), listCandidats[random].agencement.end());
					best.nbImpression.assign(listCandidats[random].nbImpression.begin(), listCandidats[random].nbImpression.end());
					best.coutTotal = listCandidats[random].coutTotal;
				}

			}


			// se finit si le temps depuis start est egal ou superieur a 60 secondes
		} while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start).count() < RUNTIME * pow(10, 6));


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
/// calcule le cout de production des couvertures
/// </summary>
/// <param name="current"></param>
/// <param name="nbEmplacement"></param>
/// <param name="coutImpression"> cout par impression</param>
/// <param name="coutFabrication"> cout par fabrication</param>
void calculCout(std::vector<unsigned int>* agencement, std::vector<unsigned int>* nbImpression, unsigned int* nbPlaques, float* coutTotal, unsigned int* nbEmplacement, float* coutImpression, float* coutFabrication) {

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
void impressionParPlaque(std::vector<unsigned int>* agencement, unsigned int* nbPlaques, std::vector<unsigned int>* nbImpressions, std::vector<unsigned int>* nbImpressionsParCouv, unsigned int* nbCouverture, unsigned int* nbEmplacement) {

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
		for (unsigned int emplacement = 0; emplacement < *nbEmplacement; emplacement++) {
			if (nbImpressions->at(plaque) < bufferIteration[agencement->at((plaque * (*nbEmplacement)) + emplacement)]) {
				nbImpressions->at(plaque) = bufferIteration[agencement->at((plaque * (*nbEmplacement)) + emplacement)];
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
bool checkValiditePlaque(std::vector<unsigned int>* agencement, unsigned int* nbCouverture) {

	try {

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
	catch (int e) {
		return false;
	}
}

/// <summary>
/// code permettant de generer l'agencement des plaques
/// </summary>
/// <param name="current"></param>
/// <param name="poidsImpression"></param>
/// <param name="nbCouverture"></param>
/// <param name="nbEmplacement"></param>
void generationPlaques(Solution* current, std::vector<float>* poidsImpression, unsigned int* nbCouverture, unsigned int* nbEmplacement) {

	int c1 = 0;
	int c2 = 0;
	float reduction = 1.0f / (float)(*nbEmplacement * current->nbPlaques);

	for (unsigned int i = 0; i < current->nbPlaques; i++) {
		for (unsigned int j = 0; j < *nbEmplacement; j++) {

			// peuplement des deux premiers
			if (poidsImpression[1] < poidsImpression[0]) {
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
void init(Solution* current, unsigned int* nbEmplacements) {

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
	for (unsigned int i = 0; i < nbImpression->size(); i++) {
		poidsImpression->at(i) += nbImpression->at(i) / total;
	}
}

/// <summary>
/// contient l'ensemble du code execute dans la partie locale de grasp, peut etre porte en multithreading
/// </summary>
/// <param name="current"></param>
/// <param name="entree"></param>
void thread(Solution* current, Entree* entree)
{
	// crée un nouvel agencement
	std::vector<unsigned int> agencementBis;
	std::vector<unsigned int> impressionsBis;
	float coutBis = 0;
	impressionsBis.assign(current->nbPlaques, 0);

	do {
		agencementBis.assign(current->agencement.begin(), current->agencement.end());
		for (int i = rand() % (entree->nbEmplacement * entree->nbCouverture); i > 0; i--) {
			agencementBis[rand() % agencementBis.size()] = (rand() % entree->nbCouverture);
		}
	} while (!checkValiditePlaque(&agencementBis, &entree->nbCouverture));

	// calcule le cout de ce nouvel agencement
	impressionParPlaque(&agencementBis, &current->nbPlaques, &impressionsBis, &entree->nbImpressionParCouverture, &entree->nbCouverture, &entree->nbEmplacement);
	calculCout(&agencementBis, &impressionsBis, &current->nbPlaques, &coutBis, &entree->nbEmplacement, &entree->coutImpression, &entree->coutFabrication);


	// compare, garde le meilleur
	if (coutBis < current->coutTotal) {

		current->agencement.assign(agencementBis.begin(), agencementBis.end());
		current->nbImpression.assign(impressionsBis.begin(), impressionsBis.end());
		current->coutTotal = coutBis;
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
			entree->nbImpressionParCouverture.push_back(stoi(inputData[ligne + 2]));
		}

		// on subdivise la dernière ligne
		int taillePremier = inputData.back().find(" ");
		entree->coutImpression = stof(inputData.back().substr(0, taillePremier));
		entree->coutFabrication = stof(inputData.back().substr(taillePremier + 1, inputData.back().find(" ")));

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
		for (unsigned int i = 0; i < solution->nbImpression.size(); i++)
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

void SwitchAgencement(Solution* current) {
	int temp = 0;
	int rand1 = 0;
	int rand2 = 0;
	int taille = current->agencement.size();

	rand1 = rand() % taille;
	rand2 = rand() % taille;
	while (rand1 == rand2) {
		rand2 = rand() % taille;
	}
	temp = current->agencement.at(rand1);
	current->agencement.at(rand1) = current->agencement.at(rand2);
	current->agencement.at(rand2) = temp;
}