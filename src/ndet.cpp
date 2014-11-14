#include "ndet.h"

// -----------------------------------------------------------------------------
// Fonctions à compléter pour la première partie du projet
// -----------------------------------------------------------------------------

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
                        for(size_t j=taillePre; j < rAutoD.trans.size(); j++){
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

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////



// -----------------------------------------------------------------------------
// Fonctions à compléter pour la seconde partie du projet
// -----------------------------------------------------------------------------

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
