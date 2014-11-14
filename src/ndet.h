#ifndef NDET_H
#define NDET_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <list>
#include <cassert>
#include <utility>
#include <cmath>

#include "tinyxml/tinyxml.h"

using namespace std;

const unsigned int ASCII_A = 97;
const unsigned int ASCII_Z = ASCII_A + 26;
const bool         DEBUG = false;

typedef size_t                            etat_t;
typedef unsigned char                     symb_t;
typedef set< etat_t >                     etatset_t;
typedef vector< vector< etatset_t > >     trans_t;
typedef vector< etatset_t >               epsilon_t;
typedef map< etatset_t, etat_t >          map_t;

struct sAutoNDE{
	// caractéristiques
	size_t nb_etats;
	size_t nb_symbs;
	size_t nb_finaux;

	etat_t initial;
	// état initial

	etatset_t finaux;
	// états finaux : finaux_t peut être un int*, un tableau dynamique comme vector<int>
	// ou une autre structure de donnée de votre choix.

	trans_t trans;
	// matrice de transition : trans_t peut être un int***, une structure dynamique 3D comme vector< vector< set<int> > >
	// ou une autre structure de donnée de votre choix.

	epsilon_t epsilon;
	// transitions spontanées : epsilon_t peut être un int**, une structure dynamique 2D comme vector< set<int> >
	// ou une autre structure de donnée de votre choix.
};


// Parseurs
bool FromFileTxt(sAutoNDE& at, string path);
bool FromFileJff(sAutoNDE& at, string path);
bool FromFile(sAutoNDE& at, string path);

// Manipulation d'automate
bool ContientFinal(const sAutoNDE& at,const etatset_t& e);
bool EstDeterministe(const sAutoNDE& at);
void Fermeture(const sAutoNDE& at, etatset_t& e);
etatset_t Delta(const sAutoNDE& at, const etatset_t& e, symb_t c);
bool Accept(const sAutoNDE& at, string str);
sAutoNDE Determinize(const sAutoNDE& at);

// Affichage
ostream& operator<<(ostream& out, const sAutoNDE& at);
bool ToGraph(sAutoNDE& at, string path);
bool ToJflap(sAutoNDE& at, string path);
ostream& operator<<(ostream& out, etatset_t e);
string toStringEtatset(etatset_t e);

sAutoNDE Append(const sAutoNDE& x, const sAutoNDE& y);
sAutoNDE Union(const sAutoNDE& x, const sAutoNDE& y);
sAutoNDE Concat(const sAutoNDE& x, const sAutoNDE& y);
sAutoNDE Complement(const sAutoNDE& x);
sAutoNDE Kleene(const sAutoNDE& x);
sAutoNDE Intersection(const sAutoNDE& x, const sAutoNDE& y);
sAutoNDE Produit(const sAutoNDE& x, const sAutoNDE& y);
sAutoNDE Minimize(const sAutoNDE& at);

#endif
