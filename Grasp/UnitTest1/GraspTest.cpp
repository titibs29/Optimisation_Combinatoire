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

			Assert::AreEqual((unsigned)0, entree.nbCouverture);
			Assert::AreEqual((unsigned)0, entree.nbEmplacement);
			Assert::AreEqual(0.0f, entree.coutImpression);
			Assert::AreEqual(0.0f, entree.coutFabrication);
		}

		TEST_METHOD(initSortie)
		{
			Solution sortie;

			Assert::AreEqual(0U, sortie.nbPlaques);
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
			Assert::AreEqual(0U, sortie.agencement[3]);
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
}