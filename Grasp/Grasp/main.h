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
void generationPlaques(Solution* current, std::vector<float>* poidsImpression, unsigned char* nbCouverture, unsigned char* nbEmplacement);
void TableauPoids(std::vector<unsigned int>* nbImpression, std::vector<float>* poidsImpression);

void SwitchAgencement(Solution* current);
void thread(Solution* current, Entree* entree);