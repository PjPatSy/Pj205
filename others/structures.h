#ifndef STRUCTURES_H_INCLUDED
#define STRUCTURES_H_INCLUDED

using namespace std;

////////////////////////////////////////////////////////////////////////////////
const unsigned int ASCII_A = 97;
const unsigned int ASCII_Z = ASCII_A + 26;
const bool         DEBUG = false;

typedef size_t                            etat_t;
typedef unsigned char                     symb_t;
typedef set<etat_t> etatset_t;              // --------- Set : Liste sans doublons
typedef vector< vector< etatset_t > >     trans_t;
typedef vector< etatset_t >               epsilon_t;
typedef map< etatset_t, etat_t >          map_t;

////////////////////////////////////////////////////////////////////////////////

struct sAutoNDE{
	// caract�ristiques
	size_t nb_etats;
	size_t nb_symbs;
	size_t nb_finaux;

	etat_t initial;
	// �tat initial

	etatset_t finaux;
	// �tats finaux : finaux_t peut �tre un int*, un tableau dynamique comme vector<int>
	// ou une autre structure de donn�e de votre choix.

	trans_t trans;
	// matrice de transition : trans_t peut �tre un int***, une structure dynamique 3D comme vector< vector< set<int> > >
	// ou une autre structure de donn�e de votre choix.

	epsilon_t epsilon;
	// transitions spontan�es : epsilon_t peut �tre un int**, une structure dynamique 2D comme vector< set<int> >
	// ou une autre structure de donn�e de votre choix.
};

#endif // STRUCTURES_H_INCLUDED
