#include "pch.h"
#include "CppUnitTest.h"
#include "../Grasp/main.h"
#include "../Grasp/main.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void show(std::vector<std::vector<float>> *matrice) {

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

	TEST_METHOD(Matrice) {

		std::vector<unsigned int> nbImpressionParCouv({ 84500,94500,103500,114500,135500 });
		std::vector<unsigned char> agencement({ 0, 4, 4, 4, 3, 0, 3, 3, 2, 1, 2, 1 });
		unsigned char  nbCouverture = 5;
		int nbPlaques = 3, nbEmplacement = 4;
		std::vector<unsigned int> nbImpression(nbPlaques, 0);
		char buffer[64];
		int colPivot = 0, lignePivot = 0;
		int iter = 0;
		float lignePlusPetit = -FLT_MAX;
		float pivot = 0;


		std::vector<std::vector<float>> matrice((nbCouverture + 1), std::vector<float>((nbPlaques + nbCouverture + 1), 0.0f));

		for (unsigned short i = 0; i < nbPlaques; i++) {
			for (unsigned short j = 0; j < nbEmplacement; j++) {
				// indices de contraintes inverses
				matrice.at(agencement.at((i * (nbEmplacement)) + j)).at(i) -= 1.0;


			}
			// ligne resultat
			matrice.at(nbCouverture).at(i) = -1;
		}

		for (int i = 0; i < nbCouverture; i++) {
			// matrice unitaire
			matrice.at(i).at(nbPlaques + i) = 1;

			// nombre d'impressions
			matrice.at(i).at(nbPlaques + nbCouverture) -= nbImpressionParCouv.at(i);
		}



		while (!estOptimal(&matrice, &nbCouverture) && iter < nbPlaques) {
			colPivot = 0;
			lignePlusPetit = FLT_MAX;

			// choix colonne pivot
			for (int i = 0; i < matrice.at(0).size(); i++)
			{

				// choix de la colonne ayant la valeur la plus faible
				if (matrice.at(nbCouverture).at(i) < matrice.at(nbCouverture).at(colPivot)) {
					colPivot = i;
				}
			}

			// choix ligne pivot
			for (int i = 0; i < matrice.size()-1; i++) {

				if (matrice.at(i).at(colPivot) !=0
					&&
					(matrice.at(i).at(nbPlaques + nbCouverture) / matrice.at(i).at(colPivot)) < lignePlusPetit) {
					
					lignePivot = i;
					lignePlusPetit = matrice.at(i).at(nbPlaques + nbCouverture) / matrice.at(i).at(colPivot);
					nbImpression.at(iter) = i;

				}
				
			}

			// pivotage
			// diviser chaque membre de la ligne du pivot par le pivot
			pivot = matrice.at(lignePivot).at(colPivot);
			for (int col = 0; col < matrice.at(0).size(); col++) {

				matrice.at(lignePivot).at(col) /= pivot;

			}


			// calcul des nouvelles cases de la matrice
			for (int ligne = 0; ligne < matrice.size(); ligne++) {
				for (int col = 0; col < matrice.at(0).size(); col++) {
					if (ligne != lignePivot && col != colPivot) {
						matrice.at(ligne).at(col) -=
							((matrice.at(lignePivot).at(col) * matrice.at(ligne).at(colPivot))
								/ matrice.at(lignePivot).at(colPivot));
					}
				}
			}

			//	// mise a zero de chaque membre de la colonne du pivot, sauf le pivot
			for (int ligne = 0; ligne < matrice.size(); ligne++) {
				if (ligne != lignePivot) {
					matrice.at(ligne).at(colPivot) = 0;
				}
			}
			iter++;

		}
		// on a la matrice optimale, on extrait les valeurs souhaitées
		for (int i = 0; i < nbPlaques; i++) {
			nbImpression.at(i) = ceil(matrice.at(nbImpression.at(i)).at(nbPlaques + nbCouverture));
			snprintf(buffer, sizeof buffer, "impressions de la plaque %d: %d \n", i, nbImpression.at(i));
			Logger::WriteMessage(buffer);
		}

		snprintf(buffer, sizeof buffer, "impressions totales: %.f \n", matrice.at(nbCouverture).at(nbPlaques + nbCouverture));
		Logger::WriteMessage(buffer);




		show(&matrice);


	}
	};

}