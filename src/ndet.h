#ifndef NDET_H
#define NDET_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <list>
#include <cassert>
#include <utility>
#include <cmath>
#include <algorithm>

#include "tinyxml/tinyxml.h"

using namespace std;

const unsigned int ASCII_A = 97;
const unsigned int ASCII_Z = ASCII_A + 26;
const bool         DEBUG = false;

typedef size_t							etat_t;
typedef unsigned char					symb_t;
typedef set< etat_t >					etatset_t;
typedef vector< vector< etatset_t > >	trans_t;
typedef vector< etatset_t >				epsilon_t;
typedef map< etatset_t, etat_t >		map_t;

struct sAutoNDE{
	// caract�ristiques
	size_t nb_etats;
	size_t nb_symbs;
	size_t nb_finaux;

	// �tat initial
	etat_t initial;

	// �tats finaux 			vector< set<int> >
	etatset_t finaux;

	// matrice de transition 	vector< vector< set<int> > >
	trans_t trans;

	// transitions spontan�es 	vector< set<int> >
	epsilon_t epsilon;
};



// -----------------------------------------------------------------------------
// PARSEURS DE FICHIERS
// -----------------------------------------------------------------------------

// charge un automate � partir d'un fichier txt ou jff
bool FromFileTxt(sAutoNDE& at, string path);

// parse un fichier txt et remplit l'automate
bool FromFile(sAutoNDE& at, string path);

// parse un fichier jff et remplit l'automate
bool FromFileJff(sAutoNDE& at, string path);



// -----------------------------------------------------------------------------
// MANIPULATION D'AUTOMATES
// -----------------------------------------------------------------------------

// indique si un ensemble d'�tats contient un des �tats finaux
bool ContientFinal(const sAutoNDE& at,const etatset_t& e);

// indique si l'automate est d�terministe
bool EstDeterministe(const sAutoNDE& at);

// renvoie l'ensemble des �tats accessibles depuis au moins un des �tats du param�tre e
etatset_t Delta(const sAutoNDE& at, const etatset_t& e, symb_t c);

// indique si l'automate accepte le mot ou pas
bool Accept(const sAutoNDE& at, string str);

// ajoute � l'ensemble d'�tats e tous ceux qui sont accessibles via des transitions spontan�es
void Fermeture(const sAutoNDE& at, etatset_t& e);

// d�terminise un automate non d�terministe comportant �ventuellement des epsilons-transitions
sAutoNDE Determinize(const sAutoNDE& at);



// -----------------------------------------------------------------------------
// AFFICHAGE
// -----------------------------------------------------------------------------

// affiche l'automate
ostream& operator<<(ostream& out, const sAutoNDE& at);

// affiche la liste d'�tats
ostream& operator<<(ostream& out, etatset_t e);

// convertit une liste d'�tat en chaine de caract�res
string toStringEtatset(etatset_t e);



// -----------------------------------------------------------------------------
// OPERATION SUR DES AUTOMATES
// -----------------------------------------------------------------------------

// regroupe deux automates en un seul
sAutoNDE Append(const sAutoNDE& x, const sAutoNDE& y);

// calcule l'union de deux automates
sAutoNDE Union(const sAutoNDE& x, const sAutoNDE& y);

// calcule la concat�nation des deux automates
sAutoNDE Concat(const sAutoNDE& x, const sAutoNDE& y);

// calcule le compl�ment d'un automate
sAutoNDE Complement(const sAutoNDE& x);

// calcule un automate *
sAutoNDE Kleene(const sAutoNDE& x);

// calcule l'intersection des deux automates par application de la loi de Do Morgan
sAutoNDE Intersection(const sAutoNDE& x, const sAutoNDE& y);

// calcule l'intersection des deux automates par produit d'automates
sAutoNDE Produit(const sAutoNDE& x, const sAutoNDE& y);



// -----------------------------------------------------------------------------
// COMPARAISON D'AUTOMATES
// -----------------------------------------------------------------------------

// d�termine la pseudo �quivalence par comparaison de tous les mots de longueur < � word_size_max
bool PseudoEquivalent(const sAutoNDE& a1, const sAutoNDE& a2, unsigned int word_size_max);

// d�termine l'�quivalence par "�galit�" des automates
bool Equivalent(const sAutoNDE& a1, const sAutoNDE& a2);

// minimise un automate avec l'algorythme de Moore
sAutoNDE Minimize(const sAutoNDE& at);



// -----------------------------------------------------------------------------
// SERIALISATION D'AUTOMATE
// -----------------------------------------------------------------------------

// enregistre un automate dans un fichier au format Graphviz
bool ToGraph(sAutoNDE& at, string path);

// enregistre un automate dans un fichier au format Jflap
bool ToJflap(sAutoNDE& at, string path);

#endif
