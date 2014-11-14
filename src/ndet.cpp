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

////////////////////////////////////////////////////////////////////////////////

const unsigned int ASCII_A = 97;
const unsigned int ASCII_Z = ASCII_A + 26;
const bool         DEBUG = false;

typedef size_t                            etat_t;
typedef unsigned char                     symb_t;
typedef set< etat_t >                     etatset_t;
typedef vector< vector< etatset_t > >     trans_t;
typedef vector< etatset_t >               epsilon_t; // --- D'un état a un ou plusieurs états
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

////////////////////////////////////////////////////////////////////////////////

bool FromFileTxt(sAutoNDE& at, string path);
bool FromFileJff(sAutoNDE& at, string path);
bool FromFile(sAutoNDE& at, string path);
bool ContientFinal(const sAutoNDE& at,const etatset_t& e);

bool EstDeterministe(const sAutoNDE& at);
void Fermeture(const sAutoNDE& at, etatset_t& e);
etatset_t Delta(const sAutoNDE& at, const etatset_t& e, symb_t c);
bool Accept(const sAutoNDE& at, string str);
sAutoNDE Determinize(const sAutoNDE& at);
ostream& operator<<(ostream& out, const sAutoNDE& at);
bool ToGraph(sAutoNDE& at, string path);
bool ToJflap(sAutoNDE& at, string path);

sAutoNDE Append(const sAutoNDE& x, const sAutoNDE& y);
sAutoNDE Union(const sAutoNDE& x, const sAutoNDE& y);
sAutoNDE Concat(const sAutoNDE& x, const sAutoNDE& y);
sAutoNDE Complement(const sAutoNDE& x);
sAutoNDE Kleene(const sAutoNDE& x);
sAutoNDE Intersection(const sAutoNDE& x, const sAutoNDE& y);
sAutoNDE Produit(const sAutoNDE& x, const sAutoNDE& y);
sAutoNDE Minimize(const sAutoNDE& at);
void Help(ostream& out, char *s);

// Utile pour afficher une liste d'état
ostream& operator<<(ostream& out, etatset_t e);
string toStringEtatset(etatset_t e);

void test();

////////////////////////////////////////////////////////////////////////////////

bool FromFile(sAutoNDE& at, string path){

	string extension;
	if (path.find_last_of(".") != std::string::npos)
		extension = path.substr(path.find_last_of(".")+1);
	else
		extension = "";
	if (extension == "txt")
		return FromFileTxt(at,path);
	if (extension == "jff")
		return FromFileJff(at,path);

	cout << "extension de fichier non reconnue\n";
	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool FromFileTxt(sAutoNDE& at, string path){
	ifstream myfile(path.c_str(), ios::in);
	//un flux d'entree obtenu à partir du nom du fichier
	string line;
	// un ligne lue dans le fichier avec getline(myfile,line);
	istringstream iss;
	// flux associé à la chaine, pour lire morceau par morceau avec >> (comme cin)
	etat_t s(0), t(0);
	// deux états temporaires
	symb_t a(0);
	// un symbole temporaire

	if (myfile.is_open()){
		// la première ligne donne 'nb_etats nb_symbs nb_finaux'
		do{
			getline(myfile,line);
		} while (line.empty() || line[0]=='#');
		// on autorise les lignes de commentaires : celles qui commencent par '#'
		iss.str(line);
		if((iss >> at.nb_etats).fail() || (iss >> at.nb_symbs).fail() || (iss >> at.nb_finaux).fail())
			return false;
		// la deuxième ligne donne l'état initial
		do{
			getline (myfile,line);
		} while (line.empty() || line[0]=='#');
		iss.clear();
		iss.str(line);
		if((iss >> at.initial).fail())
			return -1;

		// les autres lignes donnent les états finaux
		for(size_t i = 0; i < at.nb_finaux; i++){
			do{
				getline (myfile,line);
			} while (line.empty() || line[0]=='#');
			iss.clear();
			iss.str(line);
			if((iss >> s).fail())
				continue;
			//        cerr << "s= " << s << endl;
			at.finaux.insert(s);
		}

		// on alloue les vectors à la taille connue à l'avance pour éviter les resize dynamiques
		at.epsilon.resize(at.nb_etats);
		at.trans.resize(at.nb_etats);
		for(size_t i=0;i<at.nb_etats;++i)
			at.trans[i].resize(at.nb_symbs);

		// lecture de la relation de transition
		while(myfile.good()){
			line.clear();
			getline (myfile,line);
			if (line.empty() && line[0]=='#')
				continue;
			iss.clear();
			iss.str(line);

			// si une des trois lectures echoue, on passe à la suite
			if((iss >> s).fail() || (iss >> a).fail() || (iss >> t).fail() || (a< ASCII_A ) || (a> ASCII_Z ))
				continue;

			//test espilon ou non
			if ((a-ASCII_A) >= at.nb_symbs){
                at.epsilon[s].insert(t);
				cerr << "s=" << s<< ", (e), t=" << t << endl;
			}
			else{
			    at.trans[s][a-ASCII_A].insert(t);
				cerr << "s=" << s<< ", a=" << a-ASCII_A << ", t=" << t << endl;
			}
		}
		myfile.close();
		return true;
	}
	return false;
	// on ne peut pas ouvrir le fichier
}

////////////////////////////////////////////////////////////////////////////////

/* !!! Attention, les balises JFF doivent être dans le bon ordre
 * pour que cette fonction parse le fichier comme il faut */
bool FromFileJff(sAutoNDE& at, string path){
	TiXmlDocument doc(path.c_str());
	if(!doc.LoadFile()){
		return false;
	}
	TiXmlHandle hDoc(&doc);
	TiXmlElement* elem;
	elem = hDoc.FirstChildElement().Element();
	if(strcmp(elem->Value(), "structure") == 0){
		elem = elem->FirstChildElement();
		if(!elem) return false;
		if(strcmp(elem->Value(), "type") == 0){ // Type (inutile pour le moment)
				// On ne sait pas quoi faire
		}
		elem = elem->NextSiblingElement();
		// Les jff générés vias JFLAP ajoutent la balise automaton
		if(strcmp(elem->Value(), "automaton") == 0){
			elem = elem->FirstChildElement();
		}
		// Récupère les états
		at.nb_etats = 0;
		at.nb_finaux = 0;
		at.nb_symbs = 0;
		while(elem && strcmp(elem->Value(), "state") == 0){
			at.nb_etats++;
			int id = atoi(elem->Attribute("id"));
			for(TiXmlElement* tmpEl = elem->FirstChildElement(); tmpEl; tmpEl = tmpEl->NextSiblingElement()){
				if(strcmp(tmpEl->Value(), "final") == 0){
					at.nb_finaux++;
					at.finaux.insert(id);
				}
				else if(strcmp(tmpEl->Value(), "initial") == 0){
					at.initial = id;
				}
			}
			elem = elem->NextSiblingElement();
		}
		// Initialisation du nombre de symboles
		TiXmlElement* elemTrans = elem;
		while(elemTrans && strcmp(elemTrans->Value(), "transition") == 0){
			for(TiXmlElement* tmpEl = elemTrans->FirstChildElement(); tmpEl; tmpEl = tmpEl->NextSiblingElement()){
				if(strcmp(tmpEl->Value(), "read") == 0){
					if(tmpEl->GetText() != NULL){
						if(at.nb_symbs < tmpEl->GetText()[0] - ASCII_A + 1){ // Cas ou on a 'b' et pas 'a', il faut compter 'a'
							at.nb_symbs = (tmpEl->GetText()[0] - ASCII_A) + 1;
						}
					}
				}
			}
			elemTrans = elemTrans->NextSiblingElement();
		}

		// Initialisation des taille des matrices
		at.epsilon.resize(at.nb_etats);
		at.trans.resize(at.nb_etats);
		for(unsigned int i=0; i < at.nb_etats; i++){
			at.trans[i].resize(at.nb_symbs);
		}

		set<int> listSymbs; // Liste des symboles trouvés
		// Récupère le transitions
		while(elem && strcmp(elem->Value(), "transition") == 0){
			int from, to, read;
			bool epsilonTrans = false; // True si on trouve une epsilon transition
			// On admet que les balise from, to, et read sont présentes

			for(TiXmlElement* tmpEl = elem->FirstChildElement(); tmpEl; tmpEl = tmpEl->NextSiblingElement()){
				if(strcmp(tmpEl->Value(), "from") == 0){
					from = atoi(tmpEl->GetText());
				}
				else if(strcmp(tmpEl->Value(), "to") == 0){
					to = atoi(tmpEl->GetText());
				}
				else if(strcmp(tmpEl->Value(), "read") == 0){
					if(tmpEl->GetText() == NULL){
						epsilonTrans = true; // Aucun carctère à lire car epsilon transition
					}
					else{
						read = tmpEl->GetText()[0]; // Le premier caract : c'est le seul
						listSymbs.insert(read); // Si on insert 2 fois le même carct, la taille ne change pas
					}
				}
			}
			if(epsilonTrans){
				at.epsilon[from].insert(to);
			}
			else{
				at.trans[from][read-ASCII_A].insert(to);
			}
			elem = elem->NextSiblingElement();
		}
	}
	else{
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool ContientFinal(const sAutoNDE& at, const etatset_t& e){
	etatset_t::const_iterator eit(e.begin()), eend(e.end());
	for(; eit != eend; ++eit){
		etatset_t::const_iterator atit(at.finaux.begin()), atend(at.finaux.end());
		for(; atit != atend; ++atit){
			if(*eit == *atit) return true;
		}
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool EstDeterministe(const sAutoNDE& at){
    // Si on trouve une esplion transition, le graphe est non déterministe
    for(size_t i=0; i < at.epsilon.size(); i++){
        if(!at.epsilon[i].empty()){
            return false;
        }
    }

    for(size_t i=0; i < at.trans.size(); i++){
        for(size_t j=0; j < at.trans[i].size(); j++){
            // Si on a plus de 2 états pour un état de départ et une transition -> non déterministe
            // Si on a aucun états pour d'arrivé pour un état de départ et un carctère
            if(at.trans[i][j].size() != 1){
                return false;
            }
        }
    }
	return true;
}

////////////////////////////////////////////////////////////////////////////////

void Fermeture(const sAutoNDE& at, etatset_t& e){
    // Cette fonction clot l'ensemble d'états E={e_0, e_1, ... ,e_n} passé en
    // paramètre avec les epsilon transitions

    /* Il est possible d'écrire cette fonction en récursif, mais cela utilise plus de temps de calcul
    *  car on repart de la liste des états de base + les nouveau ajouter.
    *  Alors qu'ici on traite d'abords les états de base et indépendement les nouveaux ajoutés
    */

    etatset_t::iterator itEt, itEps;
    etatset_t tmp1 = e; // tmp1 : liste d'état temporaire, à chaque fois qu'on trouve
    do{
        etatset_t tmp2; // tmp2 : liste d'état, à chaque fois qu'on trouve un nouvel état par une e transition, on l'ajoute
        for(size_t i=0; i < at.epsilon.size(); i++){
            itEt = tmp1.find(i);
            if(itEt != tmp1.end() && !at.epsilon[i].empty()){
                for(itEps = at.epsilon[i].begin(); itEps != at.epsilon[i].end(); itEps++){
                    if(e.insert(*itEps).second){ // Si l'état inséré n'est pas déjà dans la liste d'états
                        tmp2.insert(*itEps);
                    }
                }
            }
        }
        tmp1 = tmp2; // On se déplace sur les nouveaux état(noeuds)
    }while(!tmp1.empty());
}

////////////////////////////////////////////////////////////////////////////////

etatset_t Delta(const sAutoNDE& at, const etatset_t& e, symb_t c){
  //TODO sur la base de celle pour le cas sans transitions spontanées,
  // définir cette fonction en utilisant Fermeture
    size_t numCar = c - ASCII_A; // Numéro du caractère
    etatset_t D_etat; // Liste des états par Delta

    if(numCar > at.nb_etats){ // Si le caractère n'est pas dans l'alphabet
        return D_etat; // Retour une liste d'état vide
    }

    for(size_t i=0; i < at.trans.size(); i++){
        if(e.find(i) != e.end() && !at.trans[i][numCar].empty()){ // l'état i est demandé et sa transision par c existe
            set<etat_t>::iterator itTrans;
            for(itTrans = at.trans[i][numCar].begin(); itTrans != at.trans[i][numCar].end(); itTrans++){
                D_etat.insert(*itTrans);
            }
        }
    }

    Fermeture(at, D_etat); // Retourne la fermeture par e-transition des états trouvés
    return D_etat;
}

////////////////////////////////////////////////////////////////////////////////

bool Accept(const sAutoNDE& at, string str){
    etatset_t tmp; // Liste d'état temporaire, qui sera changée à chaque transition
    tmp.insert(at.initial);
    Fermeture(at, tmp);
    while(!str.empty()){
        tmp = Delta(at, tmp, str[0]);
        if(tmp.empty()){ // Si aucun état n'a pour transition la lettre i, le mot n'est pas valide
            return false;
        }
        str = str.substr(1);
    }

    if(ContientFinal(at, tmp)){
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Determinize(const sAutoNDE& at){
    sAutoNDE rAutoD; // Automate déterminisé
    if(!EstDeterministe(at)){
        map_t corespEtat; // Fait la correspondance entre le numéro d'état dans rAuto, et sa les états qu'il représente dans at
        rAutoD.nb_symbs = at.nb_symbs;
        rAutoD.nb_finaux = 0;

          //////////////////////////////////////////////////////////////////////////////////////////
         ////                             Les epsilon Fermeture                                ////
        //////////////////////////////////////////////////////////////////////////////////////////
        int offset = 0; // Utile quand 2 états ont la même femeture, on ne doit rajouter qu'un seul état dans l'automate final
        cout << "Epsilon-fermeture:" << endl;
        for(size_t i=0; (i+offset) < at.nb_etats; i++){
            etatset_t tmp; // Contient l'état dont on doit calculer la fermeture
            tmp.insert(i);
            Fermeture(at, tmp);
            pair<std::map<etatset_t,etat_t>::iterator,bool> res; // Permet d'avoir un retour sur la méthode insert
            res = corespEtat.insert(pair<etatset_t, etat_t>(tmp, i+offset));
            cout << "  E(" << i << ") = " << tmp;
            if(!res.second){ // Si cette fermeture existe déjà, on ne rajoute pas d'état dans l'automate final
                cout << " = E(" << res.first->second << ")";
                i--; // Comme on a enregistré aucun état dans la map correspondant à l'état i on reste sur cet indice
                offset++; // Augmenter l'offset permet de passer l'état suivant dans at même si on est toujours sur le même état dans rAutoD
            }
            else{
                if(ContientFinal(at, tmp)){
                    rAutoD.finaux.insert(i); // Ajoute l'état actuel comme final
                }
            }
            if((i+offset) == at.initial){
                // L'état initial de M' est la E-fermeture de l'état initial de M
                // Si la E-fermeture de M est déja dans la map, l'état initial de M' à l'état déjà dans la map
                // Sinon l'état init de M' est le nouvel état ajouté (le retour de l'insert permet de réaliser cette condition)
                rAutoD.initial = res.first->second;
            }
            cout << endl;
        }

          //////////////////////////////////////////////////////////////////////////////////////////
         ////                             Delta                                                ////
        //////////////////////////////////////////////////////////////////////////////////////////
        map_t tmpMap1 = corespEtat; // Map temporaire permettant de se déplacer d'enregistrer les états dès qu'on se déplace dans l'automate
        map_t tmpMap2;
        string str = ""; // Contiendra des infos sur les transision, sera affichée à la fin de la fonction

        while(!tmpMap1.empty()){
            for(map_t::iterator itM = tmpMap1.begin(); itM != tmpMap1.end(); itM++){
                for(size_t i=0; i < rAutoD.nb_symbs; i++){
                    pair<std::map<etatset_t,etat_t>::iterator,bool> res;
                    etatset_t deltaRes = Delta(at, itM->first, (char)(i+ASCII_A));
                    res = corespEtat.insert(pair<etatset_t, etat_t>(deltaRes, corespEtat.size()));
                    /* On enregistre la transition (itM->second : numéro de l'état d'où on vient
                       i : le caractère de transition, res.first->second : l'état ou l'on va) */
                    // Redimention dynamique de la liste des transitions
                    if(itM->second >= rAutoD.trans.size()){
                        int taillePre = rAutoD.trans.size();
                        rAutoD.trans.resize(itM->second+1);
                        for(int j=taillePre; j < rAutoD.trans.size(); j++){
                            rAutoD.trans[j].resize(at.nb_symbs); // Laisse la place pour tout les symboles
                        }
                        rAutoD.trans[itM->second][i].insert(res.first->second);
                    }
                    else{
                        rAutoD.trans[itM->second][i].insert(res.first->second);
                    }
                    str += " delta(";
                    str += toStringEtatset(itM->first);
                    str += ", ";
                    str += (char)(i+ASCII_A);
                    str += ") = ";
                    str += toStringEtatset(res.first->first);
                    str += "\n";
                    if(res.second){ // Si on a un nouvel ensemble
                        tmpMap2.insert(pair<etatset_t, etat_t>(deltaRes, res.first->second));
                        if(ContientFinal(at, deltaRes)){
                            rAutoD.finaux.insert(res.first->second);
                        }
                    }
                }
            }
            tmpMap1 = tmpMap2;
            tmpMap2.clear();
        }

        rAutoD.nb_etats = corespEtat.size();
        rAutoD.nb_finaux = rAutoD.finaux.size();
        cout << endl << "Nouveau etats (" << rAutoD.nb_etats << ") :" << endl;
        for(map_t::iterator itM = corespEtat.begin(); itM != corespEtat.end(); itM++){
            cout <<itM->second << " : " << itM->first;
            if(itM->second == rAutoD.initial){
                cout << " (initial)";
            }
            if(rAutoD.finaux.find(itM->second) != rAutoD.finaux.end()){
                cout << " (final)";
            }
            cout << endl;
        }

        cout << endl << "Nouvelles transitions : " << endl;
        cout << str << endl; // Affiche les transitions
        return rAutoD;
    }
    cout << "L'automate est deja deterministe." << endl;
    return at;
}


// -----------------------------------------------------------------------------
// Fonctions à compléter pour la seconde partie du projet
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

ostream& operator<<(ostream& out, const sAutoNDE& at){
    out << endl << "Nombre etats : " << at.nb_etats << endl;
    out << "Nombre finaux : " << at.nb_finaux << endl;
    out << "Nombre symboles : " << at.nb_symbs << endl << endl;
    out << "Initial = " << at.initial << endl;
    out << "Finaux = {";
    set<etat_t>::iterator itSet;
    for(itSet = at.finaux.begin(); itSet != at.finaux.end(); itSet++){
        if(itSet != at.finaux.begin()){
            out << ",";
        }
        out << *itSet;
    }
    out << "}" << endl;
    out << "Etats = {";
    for(size_t i=0; i < at.nb_etats; i++){
        if(i != 0){
            out << ",";
        }
        out << i;
    }
    out << "}" << endl;
    out << "Alphabet = {";
    for(size_t i=0; i < at.nb_symbs; i++){
        if(i != 0){
            out << ",";
        }
        out << (char)(i+ASCII_A);
    }
    out << "}" << endl;

    out << "Transitions : " << endl;
    for(size_t i=0; i < at.trans.size(); i++){
        for(size_t j=0; j < at.trans[i].size(); j++){
            for(itSet = at.trans[i][j].begin(); itSet != at.trans[i][j].end(); itSet++){
                out << "\t" << i << " " << (char)(j+ASCII_A) << " " <<  *itSet << endl;
            }
        }
    }
    out << endl << "Ep transitions : " << endl;
    for(size_t i=0; i < at.epsilon.size(); i++){
        for(itSet = at.epsilon[i].begin(); itSet != at.epsilon[i].end(); itSet++){
            out << "\t" << i << " e " <<  *itSet << endl;
        }
    }

  return out;
}
//~

////////////////////////////////////////////////////////////////////////////////

bool ToGraph(sAutoNDE& at, string path){
  //TODO définir cette fonction

  return true;
}


////////////////////////////////////////////////////////////////////////////////

bool ToJflap(sAutoNDE& at, string path){
  //TODO définir cette fonction

  return true;
}

////////////////////////////////////////////////////////////////////////////////

// fonction outil : on garde x, et on "ajoute" trans et epsilon de y
// en renommant ses états, id est en décallant les indices des états de y
// de x.nb_etats
sAutoNDE Append(const sAutoNDE& x, const sAutoNDE& y){
  //TODO définir cette fonction
  assert(x.nb_symbs == y.nb_symbs);
  sAutoNDE r;

  return r;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Union(const sAutoNDE& x, const sAutoNDE& y){
  //TODO définir cette fonction

  assert(x.nb_symbs == y.nb_symbs);
  sAutoNDE r = Append(x, y);

  return r;
}

////////////////////////////////////////////////////////////////////////////////


sAutoNDE Concat(const sAutoNDE& x, const sAutoNDE& y){
  //TODO définir cette fonction

  assert(x.nb_symbs == y.nb_symbs);
  sAutoNDE r = Append(x, y);

  return r;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Complement(const sAutoNDE& x){
  //TODO définir cette fonction

  return x;
}


////////////////////////////////////////////////////////////////////////////////

sAutoNDE Kleene(const sAutoNDE& x){
  //TODO définir cette fonction

  return x;
}

////////////////////////////////////////////////////////////////////////////////

// Intersection avec la loi de De Morgan
sAutoNDE Intersection(const sAutoNDE& x, const sAutoNDE& y){
  //TODO définir cette fonction

  return x;
}

////////////////////////////////////////////////////////////////////////////////

// Intersection avec l'automate produit
sAutoNDE Produit(const sAutoNDE& x, const sAutoNDE& y){
  //TODO définir cette fonction

  sAutoNDE r;

  return r;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Minimize(const sAutoNDE& at){
  //TODO définir cette fonction

  assert(EstDeterministe(at));
  sAutoNDE r;

  return r;
}

////////////////////////////////////////////////////////////////////////////////

void Help(ostream& out, char *s){
  out << "Utilisation du programme " << s << " :" << endl ;
  out << "-acc ou -accept Input Word:\n\t détermine si le mot Word est accepté" << endl;
  out << "-det ou -determinize Input :\n\t déterminise Input" << endl;
  out << "-cup ou -union Input1 Input2 :\n\t calcule l'union" << endl;
  out << "-cat ou -concat Input1 Input2 :\n\t calcul la concaténation" << endl;
  out << "-star ou -kleene Input :\n\t calcul de A*" << endl;
  out << "-bar ou -complement Input :\n\t calcul du complément" << endl;
  out << "-cap ou -intersection Input1 Input2 :\n\t calcul de l'intersection par la loi de De Morgan" << endl;
  out << "-prod ou -produit Input1 Input2 :\n\t calcul de l'intersection par construction de l'automate produit" << endl;
/*
  out << "-expr2aut ou expressionrationnelle2automate ExpressionRationnelle :\n\t calcul de l'automate correspondant à l'expression rationnelle" << endl;
*/
  out << "-min ou -minimisation Input :\n\t construit l'automate standard correspondant à Input" << endl;
  out << "-nop ou -no_operation Input :\n\t ne rien faire de particulier" << endl;

  out << "-o ou -output Output :\n\t écrire le résultat dans le fichier Output, afficher sur STDOUT si non spécifié" << endl;
  out << "-g ou -graphe :\n\t l'output est au format dot/graphiz" << endl  << endl;

  out << "Exemple '" << s << " -determinize auto.txt -output determin -g'" << endl;
  out << "Exemple '" << s << " -minimisation test.jff -output min -j'" << endl;
}

////////////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[] ){

	if(argc == 2 && strcmp(argv[1], "test") == 0){
        test();
		return EXIT_SUCCESS;
	}

	if(argc < 3){
		Help(cout, argv[0]);
		return EXIT_FAILURE;
	}

	int pos;
	int act=-1;                 // pos et act pour savoir quelle action effectuer
	int nb_files = 0;           // nombre de fichiers en entrée
	string str, in1, in2, out, acc, expr;
	// chaines pour (resp.) tampon; fichier d'entrée Input1; fichier d'entrée Input2;
	// fichier de sortie et chaine dont l'acceptation est à tester
	bool toFile=false, graphMode=false, jflapMode=false;     // sortie STDOUT ou fichier ? Si fichier, format graphviz ? Jflap ?

	// options acceptées
	const size_t NBOPT = 14;
	string aLN[] = {"accept", "determinize", "union", "concat", "kleene", "complement", "intersection", "produit", "expressionrationnelle2automate", "minimisation", "no_operation", "output", "graph", "jflap"};
	string aSN[] = {"acc", "det", "cup", "cat", "star", "bar", "cap", "prod", "expr2aut", "min", "nop", "o", "g", "j"};

	// on essaie de "parser" chaque option de la ligne de commande
	for(int i=1; i<argc; ++i){
		if (DEBUG) cerr << "argv[" << i << "] = '" << argv[i] << "'" << endl;
		str = argv[i];
		pos = -1;
		string* pL = find(aLN, aLN+NBOPT, str.substr(1));
		string* pS = find(aSN, aSN+NBOPT, str.substr(1));

		if(pL!=aLN+NBOPT)
			pos = pL - aLN;
		if(pS!=aSN+NBOPT)
			pos = pS - aSN;

		if(pos != -1){
			// (pos != -1) <=> on a trouvé une option longue ou courte
			if (DEBUG) cerr << "Key found (" << pos << ") : " << str << endl;
			switch (pos) {
				case 0: //acc
					in1 = argv[++i];
					acc = argv[++i];
					nb_files = 1;
					break;
				case 1: //det
					in1 = argv[++i];
					nb_files = 1;
					break;
				case 2: //cup
					in1 = argv[++i];
					in2 = argv[++i];
					nb_files = 2;
					break;
				case 3: //cat
					in1 = argv[++i];
					in2 = argv[++i];
					nb_files = 2;
					break;
				case 4: //star
					in1 = argv[++i];
					nb_files = 1;
					break;
				case 5: //bar
					in1 = argv[++i];
					nb_files = 1;
					break;
				case 6: //cap
					in1 = argv[++i];
					in2 = argv[++i];
					nb_files = 2;
					break;
				case 7: //prod
					in1 = argv[++i];
					in2 = argv[++i];
					nb_files = 2;
					break;
				case 8: //expr2aut
					expr = argv[++i];
					nb_files = 0;
					break;
				case 9: //min
					in1 = argv[++i];
					nb_files = 1;
					break;
				case 10: //nop
					in1 = argv[++i];
					nb_files = 1;
					break;
				case 11: //o
					toFile = true;
					out = argv[++i];
					break;
				case 12: //g
					graphMode = true;
					break;
				case 13: //j
					jflapMode = true;
					break;
				default:
					return EXIT_FAILURE;
			}
		}
		else{
			cerr << "Option inconnue "<< str << endl;
			return EXIT_FAILURE;
		}

		if(pos<11){
			if(act > -1){
				cerr << "Plusieurs actions spécififées"<< endl;
				return EXIT_FAILURE;
			}
			else
				act = pos;
		}
	}

	if (act == -1){
		cerr << "Pas d'action spécififée"<< endl;
		return EXIT_FAILURE;
	}

	/* Les options sont OK, on va essayer de lire le(s) automate(s) at1 (et at2)
	et effectuer l'action spécifiée. Atr stockera le résultat*/

	sAutoNDE at1, at2, atr;

	if ((nb_files == 1 or nb_files == 2) and !FromFile(at1, in1)){
	cerr << "Erreur de lecture " << in1 << endl;
	return EXIT_FAILURE;
	}
	if (nb_files ==2 and !FromFile(at2, in2)){
	cerr << "Erreur de lecture " << in2 << endl;
	return EXIT_FAILURE;
	}

	switch(act) {
		case 0: //acc
			cout << "'" << acc << "' est accepté: " << Accept(at1, acc) << endl;
			atr = at1;
			break;
		case 1: //det
			atr = Determinize(at1);
			break;
		case 2: //cup
			atr =  Union(at1, at2);
			break;
		case 3: //cat
			atr =  Concat(at1, at2);
			break;
		case 4: //star
			atr =  Kleene(at1);
			break;
		case 5: //bar
			atr =  Complement(at1);
			break;
		case 6: //cap
			atr =  Intersection(at1, at2);
			break;
		case 7: //prod
			atr =  Produit(at1, at2);
			break;
		case 8: //expr2aut
		/*
		atr =  ExpressionRationnelle2Automate(expr);
		*/
		break;
			case 9: //minimisation
			atr =  Minimize(at1);
		break;
			case 10: //nop
			atr = at1;
			break;
		default:
			return EXIT_FAILURE;
	}

	// on affiche le résultat ou on l'écrit dans un fichier
	if(!toFile)
		cout << atr;
	else{
		if(graphMode){
			ToGraph(atr, out + ".gv");
			system(("dot -Tpng " + out + ".gv -o " + out + ".png").c_str());
		}
		if(jflapMode){
			ToJflap(atr, out + ".jff");
		}
		ofstream f((out + ".txt").c_str(), ios::trunc);
		if(f.fail())
			return EXIT_FAILURE;
		f << atr;
	}

	return EXIT_SUCCESS;
}

// Utile pour afficher une liste d'état
ostream& operator<<(ostream& out, etatset_t e){
    out << "{";
    for(etatset_t::iterator it = e.begin(); it != e.end(); it++){
        if(it != e.begin()){
            out << ",";
        }
        out << *it;
    }
    out << "}";
    return out;
}

string toStringEtatset(etatset_t e){
    string str = "{";

    for(etatset_t::iterator it = e.begin(); it != e.end(); it++){
        stringstream ss;
        if(it != e.begin()){
            str += ",";
        }
        ss << *it;
        str += ss.str();
    }
    str += "}";
    return str;
}

void test(){
	int nbAutomate = 19;
	string listeAutomate[] = {"automate_D_ex1","automate_D_ex2","automate_NDE_ex1","automate_NDE_ex2","automate_NDE_ex3","automate_NDE_ex4","automate_NDE_ex5","automate_NDE_ex6","automate_ND_ex1","automate_ND_ex2","automate_ND_ex3","automate_ND_ex4","automate_ND_ex5","automate_ND_ex6","automate_ND_ex7","automate_ND_ex8","output1","output2","output3"};

	for (int i = 0; i < nbAutomate; i++){
		sAutoNDE automateTXT, automateJFF;

		FromFile(automateTXT, "exemples/" + listeAutomate[i] + ".txt");
		system("clear");
		cout << endl << "##########   " << listeAutomate[i] << "   ##########" << endl;
		cout << "==> TXT" << automateTXT << endl;

		FromFile(automateJFF, "exemples/" + listeAutomate[i] + ".jff");
        cout << "==> JFF" << automateJFF << endl;



		etatset_t cf;
		cf.insert(1);
		cf.insert(0);
        cout << automateTXT.nb_etats << endl;
		cout << "==> ContientFinal " << cf << " : " << ContientFinal(automateTXT, cf) << endl;

		cout << "==> Deterministe : " << ((EstDeterministe(automateTXT))? "Oui" : "Non") << endl;

		cout << "==> Delta(" << automateTXT.finaux << ", a) = " << Delta(automateTXT, automateTXT.finaux, 'a') << endl;

		cout << "==> Accepte" << endl;
		cout << "\taba : " << Accept(automateTXT, "aba") << endl;
		cout << "\taaa : " << Accept(automateTXT, "aaa") << endl;
		cout << "\tbbb : " << Accept(automateTXT, "bbb") << endl;
		cout << "\tabb : " << Accept(automateTXT, "abb") << endl;

		etatset_t f;
		f.insert(1);
		f.insert(2);
		cout << "==> Fermeture " << f <<  " : ";
		Fermeture(automateTXT, f);
		cout << f <<  endl;
        cout << "==> Deterministation : " << endl;
		sAutoNDE automateDeterminise = Determinize(automateTXT);
		cout << "==> Determinise : " << ((EstDeterministe(automateDeterminise))? "Oui" : "Non")<< endl;

        cout << endl << "Appuyez sur entrer pour continuer..." << endl;
		while(getchar() != '\n');
	}
}
