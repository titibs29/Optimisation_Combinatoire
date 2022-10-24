// main.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include "main.h"

// parametres
#define RUNTIME 15				// temps de la boucle (secondes)
#define NBCANDIDATES 10			// nombre de candidats 
#define NBITERLOCAL 100			// nombre d'iteration locale
#define NBTHREADS 4				// (useless)nombre de threads
#define CHANCESMOINSPLAQUES 80	// en pourcent les chances d'avoir moins de plaques que le meilleur resultat dans le candidat
#define PARTAVIRER 0.5			// sur 1, la quantité du pool de candidats à remplacer






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
	unsigned long int iterations = 0;
	unsigned long int plaquesGenerees = 0;
	unsigned long int newBest = 0;
	unsigned long int nbTotCandidats = 0;
	unsigned long int microseconds = 0;     // duree
	bool err = false;
	float pireCout = 0.0;
	std::vector<float> poidsImpression;
	Entree entree;
	Solution best;
	Solution listCandidats[NBCANDIDATES];
	best.coutTotal = FLT_MAX;				//attribution de la valeur la plus importante possible a best

	// gestion du temps
	std::chrono::time_point<std::chrono::system_clock> start;


	try {

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
		for (i = 0; i < NBITERLOCAL; i++) {

			random = rand() % NBCANDIDATES;
			thread(&listCandidats[random], &entree);
			iterations++;    // stat
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
		std::cout << "nombre de candidats crees: " << nbTotCandidats << std::endl;




		/*-----FIN-----*/
		// ecriture du fichier de sortie
		err = ecriture(&best, &entree.nbEmplacement, &nbdataset);
		if (err) throw 99;
		system(("start notepad output/O" + std::to_string(nbdataset) + ".out").c_str()); // ouvre le fichier de sortie
		system(("start notepad Dataset-Dev/I" + std::to_string(nbdataset) + ".in").c_str()); // ouvre le fichier en entrée



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
		else
		{
			std::cout << "erreur inconnue" << std::endl;
		}

	}
}



/// <summary>
/// calcul du cout de production du candidat
/// </summary>
/// <param name="agencement"></param>
/// <param name="nbImpression"></param>
/// <param name="nbPlaques"></param>
/// <param name="nbEmplacement"></param>
/// <param name="coutTotal"></param>
/// <param name="coutImpression"></param>
/// <param name="coutFabrication"></param>
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
/// calcule le nombre d'impression de chaque plaque (non-optimise)
/// </summary>
/// <param name="agencement"></param>
/// <param name="nbImpressions"></param>
/// <param name="nbImpressionsParCouv"></param>
/// <param name="nbPlaques"></param>
/// <param name="nbCouverture"></param>
/// <param name="nbEmplacement"></param>
void impressionParPlaque(std::vector<unsigned char>* agencement,
	std::vector<unsigned int>* nbImpressions,
	std::vector<unsigned int>* nbImpressionsParCouv,
	unsigned short int* nbPlaques,
	unsigned char* nbCouverture,
	unsigned char* nbEmplacement) {

	// creation d'une matrice de la bonne taille contenant des 0
	std::vector<std::vector<float>> matrice((*nbCouverture + 1), std::vector<float>((*nbPlaques + *nbCouverture + 1), 0.0f));
	

	// creer matrice (indices contraintes inverses, matrice unitaire, impressions inversees)
	// la derniere ligne contient un -1 pour chaque plaques, un 0 sinon


	// indices de contraintes inverses
	for (unsigned short i = 0; i < *nbPlaques; i++) {
		for (unsigned short j = 0; j < *nbEmplacement; j++) {
			matrice.at(agencement->at((i * (*nbEmplacement)) + j)).at(i)-=1.0;
		}
		// ligne resultat
		matrice.at(*nbCouverture).at(i) = -1;
	}

	for (int i = 0; i < *nbCouverture; i++) {
		// matrice unitaire
		matrice.at(i).at(*nbPlaques + i) = 1;

		// nombre d'impressions
		matrice.at(i).at(*nbPlaques + *nbCouverture) -= nbImpressionsParCouv->at(i);
	}


	

	while(!estOptimal()){
		// choix colonne pivot

		// choix ligne pivot

		// pivotage
		// diviser chaque membre de la ligne du pivot par le pivot

		// calcul des nouvelles cases de la matrice
		//for (int ligne = 0; ligne < nbLigneMatrice; ligne++) {
		//	for (int col = 0; col < nbColMatrice; col++) {
		//		// case = case - (element lignePivot* element ColPivot)
		//	}
		//}

	//	// mise a zero de chaque membre de la colonne du pivot, sauf le pivot


	}
	// on a la matrice optimale, on extrait les valeurs souhaitée

}

bool estOptimal(std::vector<std::vector<float>> matrice) {
	// la matrice est optimale quand aucun element de la dernière ligne n'est inferieur a zero
	matrice.at();
	return 0;
}


void show(std::vector<std::vector<float>> matrice) {
	// cette fonction affiche la matrice
	for (int ligne = 0; ligne < matrice.size(); ligne++) {
		for (int col = 0; col < matrice[0].size(); col++) {
			std::cout << matrice[ligne][col] << " ";
		}
		std::cout << std::endl;
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


void thread(Solution* current,Entree* entree)
{
	// crée un nouvel agencement
	std::vector<unsigned char> agencementBis;
	std::vector<unsigned int> impressionsBis;
	float coutBis = 0.0;
	int i = 0;
	impressionsBis.assign(current->nbPlaques, 0);

	do {
		agencementBis.assign(current->agencement.begin(), current->agencement.end());

		for (i= rand() % agencementBis.size()+1; i > 0; i--) {
			agencementBis[rand() % agencementBis.size()] = (rand() % entree->nbCouverture);
		}

	} while (!checkValiditePlaque(&agencementBis, &entree->nbCouverture));


	// calcule le cout de ce nouvel agencement
	impressionParPlaque(&agencementBis, &impressionsBis, &entree->nbImpressionParCouverture, &current->nbPlaques, &entree->nbCouverture, &entree->nbEmplacement);
	calculCout(&agencementBis, &impressionsBis, &current->nbPlaques, &entree->nbEmplacement, &coutBis, &entree->coutImpression, &entree->coutFabrication);


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
