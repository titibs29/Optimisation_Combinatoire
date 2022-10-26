#include "pch.h"
#include "CppUnitTest.h"
#include "../Grasp/main.h"
#include "../Grasp/main.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void showLogger(std::vector<std::vector<float>> *matrice) {

	char buffer[64];
	// affiche la matrice
	for (int ligne = 0; ligne < matrice->size(); ligne++) {
		for (int col = 0; col < matrice->at(0).size(); col++) {

			snprintf(buffer, sizeof buffer, "%.f ", matrice->at(ligne).at(col));
			Logger::WriteMessage(buffer);

		}
		Logger::WriteMessage("\n");
	}
	Logger::WriteMessage("\n");
}


namespace GraspTest
{
	TEST_CLASS(initTest)
	{
	public:

		TEST_METHOD(initEntree)
		{
			Entree entree;

			Assert::AreEqual((unsigned char)0, entree.nbCouverture);
			Assert::AreEqual((unsigned char)0, entree.nbEmplacement);
			Assert::AreEqual(0.0f, entree.coutImpression);
			Assert::AreEqual(0.0f, entree.coutFabrication);
		}

		TEST_METHOD(initSortie)
		{
			Solution sortie;

			//Assert::AreEqual((unsigned short)0, sortie.nbPlaques);
			Assert::AreEqual(true, sortie.actif);
			Assert::AreEqual(FLT_MAX, sortie.coutTotal);
		}

		TEST_METHOD(initFunction)
		{
			Solution sortie;
			unsigned char nbEmplacement = 2;
			sortie.nbPlaques = 2;

			init(&sortie, &nbEmplacement);

			Assert::AreEqual(0U, sortie.nbImpression[1]);
			Assert::AreEqual((unsigned char)0, sortie.agencement[3]);
		}
	};

	TEST_CLASS(LectureTest) {

public:

	TEST_METHOD(lecture1)
	{
		Entree file1, expected;
		expected.nbCouverture = 3;
		expected.nbEmplacement = 4;
		expected.coutImpression = 13.44;
		expected.coutFabrication = 18676;
		unsigned int nbDataset = 1;

		lecture(&file1, "file1.in");

		Assert::AreEqual(expected.nbCouverture, file1.nbCouverture);
		Assert::AreEqual(expected.nbEmplacement, file1.nbEmplacement);
		Assert::AreEqual(expected.coutImpression, file1.coutImpression);
		Assert::AreEqual(expected.coutFabrication, file1.coutFabrication);
	}
	};


	TEST_CLASS(nbImpressions) {

public:

	TEST_METHOD(createNullMatrice) {

		int nbCouverture = 4, nbPlaques = 3;
		char buffer[64];

		std::vector<std::vector<float>> matrice((nbCouverture + 1), std::vector<float>((nbPlaques + nbCouverture + 1), 0.0f));


		for (int ligne = 0; ligne < matrice.size(); ligne++) {
			for (int col = 0; col < matrice[0].size(); col++) {

				snprintf(buffer, sizeof buffer, "%.f ", matrice[ligne][col]);
				Logger::WriteMessage(buffer);


			}
			Logger::WriteMessage("\n");
		}


	}
	TEST_METHOD(createMatrice) {

		int nbCouverture = 5, nbPlaques = 3, nbEmplacement = 4;
		std::vector<unsigned int> nbImpressionParCouv({ 84500,94500,103500,114500,135500 });
		std::vector<unsigned char> agencement({ 0, 4, 4, 4, 3, 0, 3, 3, 2, 1, 2, 1 });
		char buffer[64];

		std::vector<std::vector<float>> matrice((nbCouverture + 1), std::vector<float>((nbPlaques + nbCouverture + 1), 0.0f));

		for (unsigned short i = 0; i < nbPlaques; i++) {
			for (unsigned short j = 0; j < nbEmplacement; j++) {
				// indices de contraintes inverses
				matrice.at(agencement.at((i * (nbEmplacement)) + j)).at(i) -= 1.0;

				
			}
			// ligne resultat
			matrice.at(nbCouverture).at(i) = -1;
		}

		for (int i=0; i < nbCouverture; i++) {
			// matrice unitaire
			matrice.at(i).at(nbPlaques + i) = 1;

			// nombre d'impressions
			matrice.at(i).at(nbPlaques + nbCouverture) -=nbImpressionParCouv.at(i);
		}





		// affiche la matrice
		for (int ligne = 0; ligne < matrice.size(); ligne++) {
			for (int col = 0; col < matrice.at(0).size(); col++) {

				snprintf(buffer, sizeof buffer, "%.f ", matrice.at(ligne).at(col));
				Logger::WriteMessage(buffer);

			}
			Logger::WriteMessage("\n");
		}


	}

	TEST_METHOD(Matrice1) {

		std::vector<unsigned int> nbImpressionParCouv({ 84500,94500,103500,114500,135500 });
		std::vector<unsigned char> agencement({ 0, 4, 4, 4, 3, 0, 3, 3, 2, 1, 2, 1 });
		unsigned char  nbCouverture = 5;
		int nbPlaques = 3, nbEmplacement = 4;
		std::vector<unsigned int> nbImpression(nbPlaques, 0);

		char buffer[64];
		int colPivot = 0, lignePivot = 0;
		float lignePlusPetit = -FLT_MAX;
		float pivot = 0;


		std::vector<std::vector<float>> matrice(
			(nbPlaques + 1),											// nombre de lignes
			std::vector<float>((nbCouverture + nbPlaques + 1),			// nombre de colonnes
				0.0f));


		// remplissage matrice
		for (unsigned short i = 0; i < nbPlaques; i++) {
			for (unsigned short j = 0; j < nbEmplacement; j++) {
				// indices de contraintes 
				matrice.at(i).at(agencement.at((i * (nbEmplacement)) + j)) += 1.0;


			}
			// ligne resultat
			matrice.at(i).at(nbCouverture + nbPlaques) = 1;
		}

		for (int i = 0; i < nbPlaques; i++) {
			// matrice unitaire
			matrice.at(i).at(nbCouverture + i) = 1;

		}

		for (int i = 0; i < nbCouverture; i++) {

			// nombre d'impressions
			matrice.at(nbPlaques).at(i) -= nbImpressionParCouv.at(i);
		}

		Logger::WriteMessage("matrice initiale: \n");
		showLogger(&matrice);

		while (!estOptimal(&matrice)) {
			colPivot = 0;
			lignePlusPetit = FLT_MAX;

			// choix colonne pivot
			for (int i = 0; i < matrice.at(0).size(); i++)
			{

				// choix de la colonne ayant la valeur la plus faible sous le 0
				if (matrice.at(nbPlaques).at(i) < 0 &&
					matrice.at(nbPlaques).at(i) < matrice.at(nbPlaques).at(colPivot)) {
					colPivot = i;
				}
			}



			// choix ligne pivot
			for (int i = 0; i < matrice.size() - 1; i++) {
				if (matrice.at(i).at(colPivot) != 0
					&&
					(matrice.at(i).at(nbCouverture + nbPlaques) / matrice.at(i).at(colPivot)) < lignePlusPetit) {

					lignePivot = i;
					lignePlusPetit = matrice.at(i).at(nbCouverture + nbPlaques) / matrice.at(i).at(colPivot);

				}

			}

			snprintf(buffer, sizeof buffer, "coordonees pivot: %d %d \n", lignePivot, colPivot);
			Logger::WriteMessage(buffer);

			// pivotage


			pivot = matrice.at(lignePivot).at(colPivot);

			


			// calcul des nouvelles cases de la matrice
			for (int ligne = 0; ligne < matrice.size(); ligne++) {
				if (ligne != lignePivot) {

					for (int col = 0; col < matrice.at(0).size(); col++) {

						// case = case - (element ligne pivot * (element Colonne pivot / pivot))
						if (
							col != colPivot)
						{

							float step1 = matrice.at(ligne).at(colPivot) / pivot;

							float step2 = matrice.at(lignePivot).at(col) * step1;

							matrice.at(ligne).at(col) -= (step2);

						}
					}
				}
			}

			// diviser chaque membre de la ligne du pivot par le pivot
			for (int lig = 0; lig < matrice.size(); lig++) {

				matrice.at(lig).at(colPivot) /= pivot;

			}

			// mise a zero de chaque membre de la colonne du pivot, sauf le pivot
			for (int ligne = 0; ligne < matrice.size(); ligne++) {
				if (ligne != lignePivot) {
					matrice.at(ligne).at(colPivot) = 0;
				}
			}



		}

		// on a la matrice optimale, on extrait les valeurs souhaitées
		Logger::WriteMessage("matrice optimisee: \n");
		showLogger(&matrice);
		for (int i = 0; i < nbPlaques; i++) {
			nbImpression.at(i) = ceil(matrice.at(nbPlaques).at(nbCouverture + i));

			snprintf(buffer, sizeof buffer, "impressions de la plaque %d: %d \n", i, nbImpression.at(i));
			Logger::WriteMessage(buffer);
		}

		snprintf(buffer, sizeof buffer, "impressions totales: %.f \n", matrice.at(nbPlaques).at(nbCouverture + nbPlaques));
		Logger::WriteMessage(buffer);



	}

	TEST_METHOD(Matrice2) {

		std::vector<unsigned int> nbImpressionParCouv({ 4500,9000,16000 });
		std::vector<unsigned char> agencement({ 0,2,0,1, 0,0,0,0 });
		unsigned char  nbCouverture = 3;
		int nbPlaques = 2, nbEmplacement = 4;
		std::vector<unsigned int> nbImpression(nbPlaques, 0);

		char buffer[64];
		int colPivot = 0, lignePivot = 0;
		float lignePlusPetit = -FLT_MAX;
		float pivot = 0;


		std::vector<std::vector<float>> matrice(
			(nbPlaques + 1),											// nombre de lignes
			std::vector<float>((nbCouverture + nbPlaques + 1),			// nombre de colonnes
				0.0f));


		// remplissage matrice
		for (unsigned short i = 0; i < nbPlaques; i++) {
			for (unsigned short j = 0; j < nbEmplacement; j++) {
				// indices de contraintes 
				matrice.at(i).at(agencement.at((i * (nbEmplacement)) + j)) += 1.0;


			}
			// ligne resultat
			matrice.at(i).at(nbCouverture + nbPlaques) = 1;
		}

		for (int i = 0; i < nbPlaques; i++) {
			// matrice unitaire
			matrice.at(i).at(nbCouverture + i) = 1;

		}

		for (int i = 0; i < nbCouverture; i++) {

			// nombre d'impressions
			matrice.at(nbPlaques).at(i) -= nbImpressionParCouv.at(i);
		}

		Logger::WriteMessage("matrice initiale: \n");
		showLogger(&matrice);

		while (!estOptimal(&matrice)) {
			colPivot = 0;
			lignePlusPetit = FLT_MAX;

			// choix colonne pivot
			for (int i = 0; i < matrice.at(0).size(); i++)
			{

				// choix de la colonne ayant la valeur la plus faible sous le 0
				if (matrice.at(nbPlaques).at(i) < 0 &&
					matrice.at(nbPlaques).at(i) < matrice.at(nbPlaques).at(colPivot)) {
					colPivot = i;
				}
			}



			// choix ligne pivot
			for (int i = 0; i < matrice.size() - 1; i++) {
				if (matrice.at(i).at(colPivot) != 0
					&&
					(matrice.at(i).at(nbCouverture + nbPlaques) / matrice.at(i).at(colPivot)) < lignePlusPetit) {

					lignePivot = i;
					lignePlusPetit = matrice.at(i).at(nbCouverture + nbPlaques) / matrice.at(i).at(colPivot);

				}

			}

			snprintf(buffer, sizeof buffer, "coordonees pivot: %d %d \n", lignePivot, colPivot);
			Logger::WriteMessage(buffer);

			// pivotage


			pivot = matrice.at(lignePivot).at(colPivot);

			


			// calcul des nouvelles cases de la matrice
			for (int ligne = 0; ligne < matrice.size(); ligne++) {
				if (ligne != lignePivot) {

					for (int col = 0; col < matrice.at(0).size(); col++) {
						
						// case = case - (element ligne pivot * (element Colonne pivot / pivot))
						if (
							col != colPivot)
						{

							float step1 = matrice.at(ligne).at(colPivot) / pivot;
							
							float step2 = matrice.at(lignePivot).at(col) * step1;
							
							matrice.at(ligne).at(col) -= (step2);
					
						}
					}
				}
			}

			// diviser chaque membre de la ligne du pivot par le pivot
			for (int lig = 0; lig < matrice.size(); lig++) {

				matrice.at(lig).at(colPivot) /= pivot;

			}

			// mise a zero de chaque membre de la colonne du pivot, sauf le pivot
			for (int ligne = 0; ligne < matrice.size(); ligne++) {
				if (ligne != lignePivot) {
					matrice.at(ligne).at(colPivot) = 0;
				}
			}



		}

		// on a la matrice optimale, on extrait les valeurs souhaitées
		Logger::WriteMessage("matrice optimisee: \n");
		showLogger(&matrice);
		for (int i = 0; i < nbPlaques; i++) {
			nbImpression.at(i) = ceil(matrice.at(nbPlaques).at(nbCouverture + i));

			snprintf(buffer, sizeof buffer, "impressions de la plaque %d: %d \n", i, nbImpression.at(i));
			Logger::WriteMessage(buffer);
		}

		snprintf(buffer, sizeof buffer, "impressions totales: %.f \n", matrice.at(nbPlaques).at(nbCouverture + nbPlaques));
		Logger::WriteMessage(buffer);



	}


	TEST_METHOD(Matrice3) {

		std::vector<unsigned int> nbImpressionParCouv({ 4500,9000,16000 });
		std::vector<unsigned char> agencement({ 0,2,0,0, 2,2,0,2, 2,1,2,2 });
		unsigned char  nbCouverture = 3;
		int nbPlaques = 3, nbEmplacement = 4;
		std::vector<unsigned int> nbImpression(nbPlaques, 0);

		char buffer[64];
		int colPivot = 0, lignePivot = 0;
		float lignePlusPetit = -FLT_MAX;
		float pivot = 0;


		std::vector<std::vector<float>> matrice(
			(nbPlaques + 1),											// nombre de lignes
			std::vector<float>((nbCouverture + nbPlaques + 1),			// nombre de colonnes
				0.0f));


		// remplissage matrice
		for (unsigned short i = 0; i < nbPlaques; i++) {
			for (unsigned short j = 0; j < nbEmplacement; j++) {
				// indices de contraintes 
				matrice.at(i).at(agencement.at((i * (nbEmplacement)) + j)) += 1.0;


			}
			// ligne resultat
			matrice.at(i).at(nbCouverture + nbPlaques) = 1;
		}

		for (int i = 0; i < nbPlaques; i++) {
			// matrice unitaire
			matrice.at(i).at(nbCouverture + i) = 1;

		}

		for (int i = 0; i < nbCouverture; i++) {

			// nombre d'impressions
			matrice.at(nbPlaques).at(i) -= nbImpressionParCouv.at(i);
		}

		Logger::WriteMessage("matrice initiale: \n");
		showLogger(&matrice);

		while (!estOptimal(&matrice)) {
			colPivot = 0;
			lignePlusPetit = FLT_MAX;

			// choix colonne pivot
			for (int i = 0; i < matrice.at(0).size(); i++)
			{

				// choix de la colonne ayant la valeur la plus faible sous le 0
				if (matrice.at(nbPlaques).at(i) < 0 &&
					matrice.at(nbPlaques).at(i) < matrice.at(nbPlaques).at(colPivot)) {
					colPivot = i;
				}
			}



			// choix ligne pivot
			for (int i = 0; i < matrice.size() - 1; i++) {
				if (matrice.at(i).at(colPivot) != 0
					&&
					(matrice.at(i).at(nbCouverture + nbPlaques) / matrice.at(i).at(colPivot)) < lignePlusPetit) {

					lignePivot = i;
					lignePlusPetit = matrice.at(i).at(nbCouverture + nbPlaques) / matrice.at(i).at(colPivot);

				}

			}

			snprintf(buffer, sizeof buffer, "coordonees pivot: %d %d \n", lignePivot, colPivot);
			Logger::WriteMessage(buffer);

			// pivotage


			pivot = matrice.at(lignePivot).at(colPivot);




			// calcul des nouvelles cases de la matrice
			for (int ligne = 0; ligne < matrice.size(); ligne++) {
				if (ligne != lignePivot) {

					for (int col = 0; col < matrice.at(0).size(); col++) {

						// case = case - (element ligne pivot * (element Colonne pivot / pivot))
						if (
							col != colPivot)
						{

							float step1 = matrice.at(ligne).at(colPivot) / pivot;

							float step2 = matrice.at(lignePivot).at(col) * step1;

							matrice.at(ligne).at(col) -= (step2);

						}
					}
				}
			}

			// diviser chaque membre de la ligne du pivot par le pivot
			for (int lig = 0; lig < matrice.size(); lig++) {

				matrice.at(lig).at(colPivot) /= pivot;

			}

			// mise a zero de chaque membre de la colonne du pivot, sauf le pivot
			for (int ligne = 0; ligne < matrice.size(); ligne++) {
				if (ligne != lignePivot) {
					matrice.at(ligne).at(colPivot) = 0;
				}
			}

			showLogger(&matrice);

		}

		// on a la matrice optimale, on extrait les valeurs souhaitées
		Logger::WriteMessage("matrice optimisee: \n");
		showLogger(&matrice);
		for (int i = 0; i < nbPlaques; i++) {
			nbImpression.at(i) = ceil(matrice.at(nbPlaques).at(nbCouverture + i));

			snprintf(buffer, sizeof buffer, "impressions de la plaque %d: %d \n", i, nbImpression.at(i));
			Logger::WriteMessage(buffer);
		}

		snprintf(buffer, sizeof buffer, "impressions totales: %.f \n", matrice.at(nbPlaques).at(nbCouverture + nbPlaques));
		Logger::WriteMessage(buffer);



	}

	TEST_METHOD(Matrice4) {

		std::vector<unsigned int> nbImpressionParCouv({ 4500,9000,16000 });
		std::vector<unsigned char> agencement({ 0,2,0,0, 1,2,1,2, 1,0,2,1 });
		unsigned char  nbCouverture = 3;
		int nbPlaques = 3, nbEmplacement = 4;
		std::vector<unsigned int> nbImpression(nbPlaques, 0);

		char buffer[64];
		int colPivot = 0, lignePivot = 0;
		float lignePlusPetit = -FLT_MAX;
		float pivot = 0;


		std::vector<std::vector<float>> matrice(
			(nbPlaques + 1),											// nombre de lignes
			std::vector<float>((nbCouverture + nbPlaques + 1),			// nombre de colonnes
				0.0f));


		// remplissage matrice
		for (unsigned short i = 0; i < nbPlaques; i++) {
			for (unsigned short j = 0; j < nbEmplacement; j++) {
				// indices de contraintes 
				matrice.at(i).at(agencement.at((i * (nbEmplacement)) + j)) += 1.0;


			}
			// ligne resultat
			matrice.at(i).at(nbCouverture + nbPlaques) = 1;
		}

		for (int i = 0; i < nbPlaques; i++) {
			// matrice unitaire
			matrice.at(i).at(nbCouverture + i) = 1;

		}

		for (int i = 0; i < nbCouverture; i++) {

			// nombre d'impressions
			matrice.at(nbPlaques).at(i) -= nbImpressionParCouv.at(i);
		}

		Logger::WriteMessage("matrice initiale: \n");
		showLogger(&matrice);

		while (!estOptimal(&matrice)) {
			colPivot = 0;
			lignePlusPetit = FLT_MAX;

			// choix colonne pivot
			for (int i = 0; i < matrice.at(0).size(); i++)
			{

				// choix de la colonne ayant la valeur la plus faible sous le 0
				if (matrice.at(nbPlaques).at(i) < 0 &&
					matrice.at(nbPlaques).at(i) < matrice.at(nbPlaques).at(colPivot)) {
					colPivot = i;
				}
			}



			// choix ligne pivot
			for (int i = 0; i < matrice.size() - 1; i++) {
				if (matrice.at(i).at(colPivot) != 0
					&&
					(matrice.at(i).at(nbCouverture + nbPlaques) / matrice.at(i).at(colPivot)) < lignePlusPetit) {

					lignePivot = i;
					lignePlusPetit = matrice.at(i).at(nbCouverture + nbPlaques) / matrice.at(i).at(colPivot);

				}

			}

			snprintf(buffer, sizeof buffer, "coordonees pivot: %d %d \n", lignePivot, colPivot);
			Logger::WriteMessage(buffer);

			// pivotage


			pivot = matrice.at(lignePivot).at(colPivot);




			// calcul des nouvelles cases de la matrice
			for (int ligne = 0; ligne < matrice.size(); ligne++) {
				if (ligne != lignePivot) {

					for (int col = 0; col < matrice.at(0).size(); col++) {

						// case = case - (element ligne pivot * (element Colonne pivot / pivot))
						if (
							col != colPivot)
						{

							float step1 = matrice.at(ligne).at(colPivot) / pivot;

							float step2 = matrice.at(lignePivot).at(col) * step1;

							matrice.at(ligne).at(col) -= (step2);

						}
					}
				}
			}

			// diviser chaque membre de la ligne du pivot par le pivot
			for (int lig = 0; lig < matrice.size(); lig++) {

				matrice.at(lig).at(colPivot) /= pivot;

			}

			// mise a zero de chaque membre de la colonne du pivot, sauf le pivot
			for (int ligne = 0; ligne < matrice.size(); ligne++) {
				if (ligne != lignePivot) {
					matrice.at(ligne).at(colPivot) = 0;
				}
			}

			showLogger(&matrice);

		}

		// on a la matrice optimale, on extrait les valeurs souhaitées
		Logger::WriteMessage("matrice optimisee: \n");
		showLogger(&matrice);
		for (int i = 0; i < nbPlaques; i++) {
			nbImpression.at(i) = ceil(matrice.at(nbPlaques).at(nbCouverture + i));

			snprintf(buffer, sizeof buffer, "impressions de la plaque %d: %d \n", i, nbImpression.at(i));
			Logger::WriteMessage(buffer);
		}

		snprintf(buffer, sizeof buffer, "impressions totales: %.f \n", matrice.at(nbPlaques).at(nbCouverture + nbPlaques));
		Logger::WriteMessage(buffer);



	}

	};

}