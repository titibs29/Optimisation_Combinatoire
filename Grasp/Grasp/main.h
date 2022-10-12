#pragma once

#include <iostream>
#include <fstream>
#include <vector>  
#include <string>
#include <chrono>

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
void generationPlaques(Solution* current, std::vector<float>* poidsImpression, unsigned int* nbCouverture, unsigned int* nbEmplacement);
void TableauPoids(std::vector<unsigned int>* nbImpression, std::vector<float>* poidsImpression);

void SwitchAgencement(Solution* current);
void thread(Solution* current, Entree* entree);