#include "fonctions.h"
#include "structures.h"



////////////////////////////////////////////////////////////////////////////////

bool FromFileTxt(sAutoNDE& at, string path);
bool FromFileJff(sAutoNDE& at, string path);

bool FromFile(sAutoNDE& at, string path){

	string extension;
	if (path.find_last_of(".") != std::string::npos)
		extension = path.substr(path.find_last_of(".") + 1);
	else
		extension = "";

	if (extension == "txt")
		return FromFileTxt(at, path);
	if (extension == "jff")
		return FromFileTxt(at, path);

	cout << "extension de fichier non reconnue\n";
	return false;
}

bool FromFileTxt(sAutoNDE& at, string path){

	ifstream myfile(path.c_str(), ios::in);
	//un flux d'entree obtenu � partir du nom du fichier
	string line;
	// un ligne lue dans le fichier avec getline(myfile,line);
	istringstream iss;
	// flux associ� � la chaine, pour lire morceau par morceau avec >> (comme cin)
	etat_t s(0), t(0);
	// deux �tats temporaires
	symb_t a(0);
	// un symbole temporaire

	if (myfile.is_open()){
		// la premi�re ligne donne 'nb_etats nb_symbs nb_finaux'
		do{
			getline(myfile, line);
		} while (line.empty() || line[0] == '#');
		// on autorise les lignes de commentaires : celles qui commencent par '#'
		iss.str(line);
		if ((iss >> at.nb_etats).fail() || (iss >> at.nb_symbs).fail() || (iss >> at.nb_finaux).fail())
			return false;
		// la deuxi�me ligne donne l'�tat initial
		do{
			getline(myfile, line);
		} while (line.empty() || line[0] == '#');
		iss.clear();
		iss.str(line);
		if ((iss >> at.initial).fail())
			return -1;

		// les autres lignes donnent les �tats finaux
		for (size_t i = 0; i < at.nb_finaux; i++){
			do{
				getline(myfile, line);
			} while (line.empty() || line[0] == '#');
			iss.clear();
			iss.str(line);
			if ((iss >> s).fail())
				continue;
			//        cerr << "s= " << s << endl;
			at.finaux.insert(s);
		}

		// on alloue les vectors � la taille connue � l'avance pour �viter les resize dynamiques
		at.epsilon.resize(at.nb_etats);
		at.trans.resize(at.nb_etats);
		for (size_t i = 0; i<at.nb_etats; ++i)
			at.trans[i].resize(at.nb_symbs);

		// lecture de la relation de transition
		while (myfile.good()){
			line.clear();
			getline(myfile, line);
			if (line.empty() && line[0] == '#')
				continue;
			iss.clear();
			iss.str(line);

			// si une des trois lectures echoue, on passe � la suite
			if ((iss >> s).fail() || (iss >> a).fail() || (iss >> t).fail() || (a< ASCII_A) || (a> ASCII_Z))
				continue;

			//test espilon ou non
			if ((a - ASCII_A) >= at.nb_symbs){
				cerr << "s=" << s << ", (e), t=" << t << endl;
				//TODO remplir epsilon
			}
			else{
				cerr << "s=" << s << ", a=" << a - ASCII_A << ", t=" << t << endl;
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
// Fonctions � compl�ter pour la premi�re partie du projet
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

bool FromFileJff(sAutoNDE& at, string path){
	//TODO d�finir cette fonction

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool ContientFinal(const sAutoNDE& at, const etatset_t& e){
	//TODO d�finir cette fonction

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool EstDeterministe(const sAutoNDE& at){
	//TODO d�finir cette fonction

	return true;
}

////////////////////////////////////////////////////////////////////////////////

void Fermeture(const sAutoNDE& at, etatset_t& e){
	// Cette fonction clot l'ensemble d'�tats E={e_0, e_1, ... ,e_n} pass� en
	// param�tre avec les epsilon transitions

	//TODO d�finir cette fonction
}

////////////////////////////////////////////////////////////////////////////////

etatset_t Delta(const sAutoNDE& at, const etatset_t& e, symb_t c){
	//TODO sur la base de celle pour le cas sans transitions spontan�es,
	// d�finir cette fonction en utilisant Fermeture

	return e;
}

////////////////////////////////////////////////////////////////////////////////

bool Accept(const sAutoNDE& at, string str){
	//TODO d�finir cette fonction

	return false;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Determinize(const sAutoNDE& at){
	//TODO d�finir cette fonction

	sAutoNDE r;
	return r;
}


// -----------------------------------------------------------------------------
// Fonctions � compl�ter pour la seconde partie du projet
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

ostream& operator<<(ostream& out, const sAutoNDE& at){
	//TODO d�finir cette fonction

	return out;
}


////////////////////////////////////////////////////////////////////////////////

bool ToGraph(sAutoNDE& at, string path){
	//TODO d�finir cette fonction

	return true;
}


////////////////////////////////////////////////////////////////////////////////

bool ToJflap(sAutoNDE& at, string path){
	//TODO d�finir cette fonction

	return true;
}

////////////////////////////////////////////////////////////////////////////////

// fonction outil : on garde x, et on "ajoute" trans et epsilon de y
// en renommant ses �tats, id est en d�callant les indices des �tats de y
// de x.nb_etats
sAutoNDE Append(const sAutoNDE& x, const sAutoNDE& y){
	//TODO d�finir cette fonction
	assert(x.nb_symbs == y.nb_symbs);
	sAutoNDE r;

	return r;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Union(const sAutoNDE& x, const sAutoNDE& y){
	//TODO d�finir cette fonction

	assert(x.nb_symbs == y.nb_symbs);
	sAutoNDE r = Append(x, y);

	return r;
}

////////////////////////////////////////////////////////////////////////////////


sAutoNDE Concat(const sAutoNDE& x, const sAutoNDE& y){
	//TODO d�finir cette fonction

	assert(x.nb_symbs == y.nb_symbs);
	sAutoNDE r = Append(x, y);

	return r;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Complement(const sAutoNDE& x){
	//TODO d�finir cette fonction

	return x;
}


////////////////////////////////////////////////////////////////////////////////

sAutoNDE Kleene(const sAutoNDE& x){
	//TODO d�finir cette fonction

	return x;
}

////////////////////////////////////////////////////////////////////////////////

// Intersection avec la loi de De Morgan
sAutoNDE Intersection(const sAutoNDE& x, const sAutoNDE& y){
	//TODO d�finir cette fonction

	return x;
}

////////////////////////////////////////////////////////////////////////////////

// Intersection avec l'automate produit
sAutoNDE Produit(const sAutoNDE& x, const sAutoNDE& y){
	//TODO d�finir cette fonction

	sAutoNDE r;

	return r;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Minimize(const sAutoNDE& at){
	//TODO d�finir cette fonction

	assert(EstDeterministe(at));
	sAutoNDE r;

	return r;
}

////////////////////////////////////////////////////////////////////////////////

void Help(ostream& out, char *s){
	out << "Utilisation du programme " << s << " :" << endl;
	out << "-acc ou -accept Input Word:\n\t d�termine si le mot Word est accept�" << endl;
	out << "-det ou -determinize Input :\n\t d�terminise Input" << endl;
	out << "-cup ou -union Input1 Input2 :\n\t calcule l'union" << endl;
	out << "-cat ou -concat Input1 Input2 :\n\t calcul la concat�nation" << endl;
	out << "-star ou -kleene Input :\n\t calcul de A*" << endl;
	out << "-bar ou -complement Input :\n\t calcul du compl�ment" << endl;
	out << "-cap ou -intersection Input1 Input2 :\n\t calcul de l'intersection par la loi de De Morgan" << endl;
	out << "-prod ou -produit Input1 Input2 :\n\t calcul de l'intersection par construction de l'automate produit" << endl;
	/*
	out << "-expr2aut ou expressionrationnelle2automate ExpressionRationnelle :\n\t calcul de l'automate correspondant � l'expression rationnelle" << endl;
	*/
	out << "-min ou -minimisation Input :\n\t construit l'automate standard correspondant � Input" << endl;
	out << "-nop ou -no_operation Input :\n\t ne rien faire de particulier" << endl;

	out << "-o ou -output Output :\n\t �crire le r�sultat dans le fichier Output, afficher sur STDOUT si non sp�cifi�" << endl;
	out << "-g ou -graphe :\n\t l'output est au format dot/graphiz" << endl << endl;

	out << "Exemple '" << s << " -determinize auto.txt -output determin -g'" << endl;
	out << "Exemple '" << s << " -minimisation test.jff -output min -j'" << endl;
}



////////////////////////////////////////////////////////////////////////////////





