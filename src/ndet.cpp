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


using namespace std;

////////////////////////////////////////////////////////////////////////////////

const unsigned int ASCII_A = 97;
const unsigned int ASCII_Z = ASCII_A + 26;
const bool         DEBUG = false;

typedef size_t                            etat_t;
typedef unsigned char                     symb_t;
typedef set< etat_t >                     etatset_t;
typedef vector< vector< etatset_t > >     trans_t;
typedef vector< etatset_t >               epsilon_t;
typedef map< etatset_t, etat_t >          map_t;


////////////////////////////////////////////////////////////////////////////////

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
				cerr << "s=" << s<< ", (e), t=" << t << endl;
				//TODO remplir epsilon
			}
			else{
				cerr << "s=" << s<< ", a=" << a-ASCII_A << ", t=" << t << endl;
				//TODO remplir trans
			}
		}
		myfile.close();
		return true; 
	}
	return false;
	// on ne peut pas ouvrir le fichier
}


// -----------------------------------------------------------------------------
// Fonctions à compléter pour la première partie du projet
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

bool FromFileJff(sAutoNDE& at, string path){
	//TODO définir cette fonction

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool ContientFinal(const sAutoNDE& at,const etatset_t& e){
	etatset_t::const_iterator eit(e.begin()), eend(e.end()); 
	for(; eit != eend; ++eit){
		etatset_t::const_iterator atit(at.finaux.begin()), atend(at.finaux.end());
		for(; atit != atend; ++atit){
			if(eit == atit) return true;
		}
	} 

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool EstDeterministe(const sAutoNDE& at){
	//TODO définir cette fonction

	return true;
}

////////////////////////////////////////////////////////////////////////////////

void Fermeture(const sAutoNDE& at, etatset_t& e){
  // Cette fonction clot l'ensemble d'états E={e_0, e_1, ... ,e_n} passé en
  // paramètre avec les epsilon transitions

  //TODO définir cette fonction
}

////////////////////////////////////////////////////////////////////////////////

etatset_t Delta(const sAutoNDE& at, const etatset_t& e, symb_t c){
  //TODO sur la base de celle pour le cas sans transitions spontanées,
  // définir cette fonction en utilisant Fermeture

  return e;
}

////////////////////////////////////////////////////////////////////////////////

bool Accept(const sAutoNDE& at, string str){
  //TODO définir cette fonction

  return false;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Determinize(const sAutoNDE& at){
  //TODO définir cette fonction

  sAutoNDE r;
  return r;
}


// -----------------------------------------------------------------------------
// Fonctions à compléter pour la seconde partie du projet
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

ostream& operator<<(ostream& out, const sAutoNDE& at){
  //TODO définir cette fonction
  
  return out;
}


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
	sAutoNDE graphe_a, graphe_b;
	FromFileTxt(graphe_a, "exemples/automate_D_ex1.txt");
	FromFileTxt(graphe_b, "exemples/automate_D_ex2.txt");
	
	cout << ContientFinal(graphe_a, graphe_b.finaux) << endl;
	cout << ContientFinal(graphe_b, graphe_a.finaux) << endl;
	cout << ContientFinal(graphe_b, graphe_b.finaux) << endl;
	
	
  //~ if(argc < 3){
    //~ Help(cout, argv[0]);
    //~ return EXIT_FAILURE;
  //~ }
  //~ 
  //~ int pos;
  //~ int act=-1;                 // pos et act pour savoir quelle action effectuer
  //~ int nb_files = 0;           // nombre de fichiers en entrée
  //~ string str, in1, in2, out, acc, expr;
  //~ // chaines pour (resp.) tampon; fichier d'entrée Input1; fichier d'entrée Input2;
  //~ // fichier de sortie et chaine dont l'acceptation est à tester 
  //~ bool toFile=false, graphMode=false, jflapMode=false;     // sortie STDOUT ou fichier ? Si fichier, format graphviz ? Jflap ?
//~ 
  //~ // options acceptées
  //~ const size_t NBOPT = 14;
  //~ string aLN[] = {"accept", "determinize", "union", "concat", "kleene", "complement", "intersection", "produit", "expressionrationnelle2automate", "minimisation", "no_operation", "output", "graph", "jflap"};
  //~ string aSN[] = {"acc", "det", "cup", "cat", "star", "bar", "cap", "prod", "expr2aut", "min", "nop", "o", "g", "j"};
  //~ 
  //~ // on essaie de "parser" chaque option de la ligne de commande
  //~ for(int i=1; i<argc; ++i){
    //~ if (DEBUG) cerr << "argv[" << i << "] = '" << argv[i] << "'" << endl;
    //~ str = argv[i];
    //~ pos = -1;
    //~ string* pL = find(aLN, aLN+NBOPT, str.substr(1));
    //~ string* pS = find(aSN, aSN+NBOPT, str.substr(1));
    //~ 
    //~ if(pL!=aLN+NBOPT)
      //~ pos = pL - aLN;
    //~ if(pS!=aSN+NBOPT)
      //~ pos = pS - aSN;   
      //~ 
    //~ if(pos != -1){
      //~ // (pos != -1) <=> on a trouvé une option longue ou courte
      //~ if (DEBUG) cerr << "Key found (" << pos << ") : " << str << endl;
      //~ switch (pos) {
        //~ case 0: //acc
          //~ in1 = argv[++i];
          //~ acc = argv[++i];
	  //~ nb_files = 1;
          //~ break;
        //~ case 1: //det
          //~ in1 = argv[++i];
	  //~ nb_files = 1;
          //~ break;
        //~ case 2: //cup
          //~ in1 = argv[++i];
          //~ in2 = argv[++i];
	  //~ nb_files = 2;
          //~ break;
        //~ case 3: //cat
          //~ in1 = argv[++i];
          //~ in2 = argv[++i];
	  //~ nb_files = 2;
          //~ break;
        //~ case 4: //star
          //~ in1 = argv[++i];
	  //~ nb_files = 1;
          //~ break;
        //~ case 5: //bar
          //~ in1 = argv[++i];
	  //~ nb_files = 1;
          //~ break;
        //~ case 6: //cap
          //~ in1 = argv[++i];
          //~ in2 = argv[++i];
	  //~ nb_files = 2;
          //~ break;
        //~ case 7: //prod
          //~ in1 = argv[++i];
          //~ in2 = argv[++i];
	  //~ nb_files = 2;
          //~ break;
        //~ case 8: //expr2aut
          //~ expr = argv[++i];
	  //~ nb_files = 0;
          //~ break;
        //~ case 9: //min
          //~ in1 = argv[++i];
	  //~ nb_files = 1;
          //~ break;
        //~ case 10: //nop
          //~ in1 = argv[++i];
	  //~ nb_files = 1;
          //~ break;          
        //~ case 11: //o
          //~ toFile = true;
          //~ out = argv[++i];
          //~ break;
        //~ case 12: //g
          //~ graphMode = true;
          //~ break;
        //~ case 13: //j
          //~ jflapMode = true;
          //~ break;
        //~ default:
          //~ return EXIT_FAILURE;
        //~ }
    //~ }
    //~ else{
      //~ cerr << "Option inconnue "<< str << endl;
      //~ return EXIT_FAILURE;
    //~ }
    //~ 
    //~ if(pos<11){
      //~ if(act > -1){
        //~ cerr << "Plusieurs actions spécififées"<< endl;
        //~ return EXIT_FAILURE;
      //~ }
      //~ else
        //~ act = pos;
    //~ }    
  //~ }
  //~ 
  //~ if (act == -1){
    //~ cerr << "Pas d'action spécififée"<< endl;
    //~ return EXIT_FAILURE;
  //~ }  
//~ 
//~ /* Les options sont OK, on va essayer de lire le(s) automate(s) at1 (et at2)
//~ et effectuer l'action spécifiée. Atr stockera le résultat*/
//~ 
  //~ sAutoNDE at1, at2, atr;
  //~ 
  //~ if ((nb_files == 1 or nb_files == 2) and !FromFile(at1, in1)){
    //~ cerr << "Erreur de lecture " << in1 << endl;
    //~ return EXIT_FAILURE;
  //~ }  
  //~ if (nb_files ==2 and !FromFile(at2, in2)){
    //~ cerr << "Erreur de lecture " << in2 << endl;
    //~ return EXIT_FAILURE;
  //~ }  
  //~ 
  //~ switch(act) {
  //~ case 0: //acc
    //~ cout << "'" << acc << "' est accepté: " << Accept(at1, acc) << endl;
    //~ atr = at1;
    //~ break;
  //~ case 1: //det
    //~ atr = Determinize(at1);
    //~ break;
  //~ case 2: //cup
    //~ atr =  Union(at1, at2); 
    //~ break;
  //~ case 3: //cat
    //~ atr =  Concat(at1, at2); 
    //~ break;
  //~ case 4: //star
    //~ atr =  Kleene(at1);
    //~ break;
  //~ case 5: //bar
    //~ atr =  Complement(at1);
    //~ break;
  //~ case 6: //cap
    //~ atr =  Intersection(at1, at2);
    //~ break;
  //~ case 7: //prod
    //~ atr =  Produit(at1, at2);
    //~ break;
  //~ case 8: //expr2aut
//~ /*
    //~ atr =  ExpressionRationnelle2Automate(expr);
//~ */
    //~ break;
  //~ case 9: //minimisation
    //~ atr =  Minimize(at1);
    //~ break;
  //~ case 10: //nop
    //~ atr = at1;
    //~ break;
  //~ default:
    //~ return EXIT_FAILURE;
  //~ }
//~ 
  //~ // on affiche le résultat ou on l'écrit dans un fichier
  //~ if(!toFile)
    //~ cout << atr;
  //~ else{
    //~ if(graphMode){
      //~ ToGraph(atr, out + ".gv");
      //~ system(("dot -Tpng " + out + ".gv -o " + out + ".png").c_str());
    //~ }
    //~ if(jflapMode){
      //~ ToJflap(atr, out + ".jff");
    //~ }
    //~ ofstream f((out + ".txt").c_str(), ios::trunc); 
    //~ if(f.fail())
      //~ return EXIT_FAILURE;
    //~ f << atr;    
  //~ }
  
  return EXIT_SUCCESS;
}



