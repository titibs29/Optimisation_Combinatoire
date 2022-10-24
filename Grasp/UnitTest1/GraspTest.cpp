#include "pch.h"
#include "CppUnitTest.h"
#include "../Grasp/main.h"
#include "../Grasp/main.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

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
			for (int col = 0; col < matrice[0].size(); col++) {

				snprintf(buffer, sizeof buffer, "%.f ", matrice[ligne][col]);
				Logger::WriteMessage(buffer);

			}
			Logger::WriteMessage("\n");
		}


	}
	};

}