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
	// un flux d'entree obtenu à partir du nom du fichier
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
			//cerr << "s= " << s << endl;
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
				cerr << "s=" << s << ", (e), t=" << t << endl;
			}
			else{
			    at.trans[s][a-ASCII_A].insert(t);
				cerr << "s=" << s << ", a=" << a-ASCII_A << ", t=" << t << endl;
			}
		}
		myfile.close();
		return true;
	}
	// on ne peut pas ouvrir le fichier
	return false;
}

////////////////////////////////////////////////////////////////////////////////

// ATTENTION les balises JFF doivent être dans le bon ordre
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
		if(strcmp(elem->Value(), "type") == 0){ // inutile pour le moment
				// on ne sait pas quoi faire
		}
		elem = elem->NextSiblingElement();
		// les jff générés vias JFLAP ajoutent la balise automaton
		if(strcmp(elem->Value(), "automaton") == 0){
			elem = elem->FirstChildElement();
		}

		// récupère les états
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

		// initialisation du nombre de symboles
		TiXmlElement* elemTrans = elem;
		while(elemTrans && strcmp(elemTrans->Value(), "transition") == 0){
			for(TiXmlElement* tmpEl = elemTrans->FirstChildElement(); tmpEl; tmpEl = tmpEl->NextSiblingElement()){
				if(strcmp(tmpEl->Value(), "read") == 0){
					if(tmpEl->GetText() != NULL){
						if(at.nb_symbs < tmpEl->GetText()[0] - ASCII_A + 1){ // cas ou on a 'b' et pas 'a', il faut compter 'a'
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

		set<int> listSymbs; // liste des symboles trouvés
		// on récupère les transitions
		while(elem && strcmp(elem->Value(), "transition") == 0){
			int from, to, read;
			bool epsilonTrans = false; // true si on trouve une epsilon transition

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
						epsilonTrans = true; // aucun carctère à lire car epsilon transition
					}
					else{
						read = tmpEl->GetText()[0]; // le premier caract car c'est le seul
						listSymbs.insert(read); // si on insert 2 fois le même carct, la taille ne change pas
					}
				}
			}

			//test espilon ou non
			if(epsilonTrans){
				at.epsilon[from].insert(to);
				cerr << "s=" << from << ", (e), t=" << to << endl;
			}
			else{
				at.trans[from][read-ASCII_A].insert(to);
				cerr << "s=" << from << ", a=" << read-ASCII_A << ", t=" << to << endl;
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
    // si on trouve une esplion transition, le graphe est non déterministe
    for(size_t i=0; i < at.epsilon.size(); i++){
        if(!at.epsilon[i].empty()){
            return false;
        }
    }

    // si on a plus de 2 états pour un état de départ et une transition, le graphe est non déterministe
    for(size_t i=0; i < at.trans.size(); i++){
        for(size_t j=0; j < at.trans[i].size(); j++){
            if(at.trans[i][j].size() != 1){
                return false;
            }
        }
    }
	return true;
}

////////////////////////////////////////////////////////////////////////////////

etatset_t Delta(const sAutoNDE& at, const etatset_t& e, symb_t c){
    size_t numCar = c - ASCII_A; 	// numéro du caractère
    etatset_t D_etat; 				// liste des états par Delta

	// si le caractère n'est pas dans l'alphabet, retour d'une liste d'état vide
    if(numCar > at.nb_etats){
        return D_etat;
    }

    for(size_t i=0; i < at.trans.size(); i++){
        if(e.find(i) != e.end() && !at.trans[i][numCar].empty()){ // l'état i est demandé et sa transision par c existe
            set<etat_t>::iterator itTrans;
            for(itTrans = at.trans[i][numCar].begin(); itTrans != at.trans[i][numCar].end(); itTrans++){
                D_etat.insert(*itTrans);
            }
        }
    }
	// retourne la fermeture par e-transition des états trouvés
    Fermeture(at, D_etat);
    return D_etat;
}

////////////////////////////////////////////////////////////////////////////////

bool Accept(const sAutoNDE& at, string str){
    etatset_t tmp; // liste d'état temporaire, sera changée à chaque transition
    tmp.insert(at.initial);
    Fermeture(at, tmp);
    while(!str.empty()){
        tmp = Delta(at, tmp, str[0]);
        // si aucun état n'a pour transition la lettre i, le mot n'est pas valide
        if(tmp.empty()){
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

void Fermeture(const sAutoNDE& at, etatset_t& e){
	// possible d'écrire cette fonction en récursif, mais utilise plus de temps de calcul
    // car on repart de la liste des états de base avec les nouveau ajouter
    // ici on traite d'abords les états de base et indépendement les nouveaux ajoutés
    etatset_t::iterator itEt, itEps;
    etatset_t tmp1 = e; // liste d'état temporaire, à chaque fois qu'on trouve un état
    do{
        etatset_t tmp2; // liste d'état, à chaque fois qu'on trouve un état par une e-transition
        for(size_t i=0; i < at.epsilon.size(); i++){
            itEt = tmp1.find(i);
            if(itEt != tmp1.end() && !at.epsilon[i].empty()){
                for(itEps = at.epsilon[i].begin(); itEps != at.epsilon[i].end(); itEps++){
                    if(e.insert(*itEps).second){
						// si l'état inséré n'est pas déjà dans la liste d'états
                        tmp2.insert(*itEps);
                    }
                }
            }
        }
        // On se déplace sur les nouveaux état
        tmp1 = tmp2;
    }while(!tmp1.empty());
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Determinize(const sAutoNDE& at){
	cout << at;
    sAutoNDE rAutoD; // automate déterminisé

    if(!EstDeterministe(at)){
        map_t corespEtat; // fait la correspondance entre le numéro d'état dans rAutoD, et les états qu'il représente dans at
        map_t fermeture; // pas très utile, seulement pour l'affichage (Pas obligatoire)
        rAutoD.nb_symbs = at.nb_symbs;
        rAutoD.nb_finaux = 0;

          //////////////////////////////////////////////////////////////////////////////////////////
         ////                             Les epsilon Fermeture                                ////
        //////////////////////////////////////////////////////////////////////////////////////////
        cout << "Epsilon-fermeture:" << endl;
        for(size_t i=0; i < at.nb_etats; i++){
            etatset_t tmp; // contient l'état dont on doit calculer la fermeture
            tmp.insert(i);
            Fermeture(at, tmp);
            pair<std::map<etatset_t,etat_t>::iterator,bool> res; // permet d'avoir un retour sur la méthode insert
            res = fermeture.insert(pair<etatset_t, etat_t>(tmp, i));
            cout << "   E(" << i << ") = " << tmp;
            // si cette fermeture existe déjà, on ne rajoute pas d'état dans l'automate final
            if(!res.second){
                cout << " = E(" << res.first->second << ")"; // map_t fermeture sert seulement à cet affichage, n'est pas très utile
            }
            else if(i == at.initial){
                corespEtat.insert(pair<etatset_t, etat_t>(tmp, 0)); // on ajoute seulement l'état initial
                // seulement l'élément initial est testé si final, les autres états seront testés après(Pour éviter aussi que la poubelle sooit ajoutée en final dans certains cas)
                if(ContientFinal(at, tmp)){
                    rAutoD.finaux.insert(0); // ajoute l'état actuel comme final
                }
                // l'état initial de M' est la e-fermeture de l'état initial de M
                // si la e-fermeture de M est déja dans la map, l'état initial de M' à l'état déjà dans la map
                // sinon l'état init de M' est le nouvel état ajouté, le retour de l'insert permet de réaliser cette condition
                rAutoD.initial = 0;
            }
            cout << endl;
        }

          //////////////////////////////////////////////////////////////////////////////////////////
         ////                             Delta                                                ////
        //////////////////////////////////////////////////////////////////////////////////////////
        map_t tmpMap1 = corespEtat;	// map temporaire permettant d'enregistrer les états dès qu'on se déplace dans l'automate
        map_t tmpMap2;
        string str = ""; 			// contiendra des infos sur les transisions, sera affichée à la fin de la fonction

        while(!tmpMap1.empty()){
            for(map_t::iterator itM = tmpMap1.begin(); itM != tmpMap1.end(); itM++){
                for(size_t i=0; i < rAutoD.nb_symbs; i++){
                    pair<std::map<etatset_t,etat_t>::iterator,bool> res;
                    etatset_t deltaRes = Delta(at, itM->first, (char)(i+ASCII_A));
                    res = corespEtat.insert(pair<etatset_t, etat_t>(deltaRes, corespEtat.size()));
                    // on enregistre la transition et redimentionnement dynamique de la liste des transitions
                    // itM->second : numéro de l'état d'où on vient
                    // i : le caractère de transition
                    // res.first->second : l'état ou l'on va
                    if(itM->second >= rAutoD.trans.size()){
                        int taillePre = rAutoD.trans.size();
                        rAutoD.trans.resize(itM->second+1);
                        for(size_t j=taillePre; j < rAutoD.trans.size(); j++){
                            rAutoD.trans[j].resize(at.nb_symbs); // laisse la place pour tout les symboles
                        }
                        rAutoD.trans[itM->second][i].insert(res.first->second);
                    }
                    else{
                        rAutoD.trans[itM->second][i].insert(res.first->second);
                    }
                    str += "   delta(";
                    str += toStringEtatset(itM->first);
                    str += ", ";
                    str += (char)(i+ASCII_A);
                    str += ") = ";
                    str += toStringEtatset(res.first->first);
                    str += "\n";
                    if(res.second){ // si on a un nouvel ensemble
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
            cout << "   " << itM->second << " : " << itM->first;
            if(itM->second == rAutoD.initial){
                cout << " (initial)";
            }
            if(rAutoD.finaux.find(itM->second) != rAutoD.finaux.end()){
                cout << " (final)";
            }
            cout << endl;
        }

        cout << endl << "Nouvelles transitions : " << endl;
        cout << str << endl; // affiche les transitions
        rAutoD.epsilon.resize(rAutoD.nb_etats); // prend de la place, mais simplifie s'il faut faire des oprérations sur les epsilon après
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

// -----------------------------------------------------------------------------
// Fonctions à compléter pour la seconde partie du projet
// -----------------------------------------------------------------------------

// on garde x, et on "ajoute" trans et epsilon de y en renommant ses états
// id est calculé en décallant les indices des états de y de x.nb_etats
sAutoNDE Append(const sAutoNDE& x, const sAutoNDE& y){
    sAutoNDE r = x;
    r.nb_etats += y.nb_etats;
    r.nb_finaux += y.nb_finaux;
    r.nb_symbs = ((y.nb_symbs > x.nb_symbs)? y.nb_symbs : x.nb_symbs);
    for(etatset_t::iterator it = y.finaux.begin(); it != y.finaux.end(); it++){
       r.finaux.insert(x.nb_etats + *it);
    }
    r.epsilon.resize(r.nb_etats);
    r.trans.resize(r.nb_etats);
    // resize (i = 0 si y à plus de symboles que x et i = nombre d'état de x sinon, pour resize seulement les trans de y dans r
    for(size_t i=((r.nb_symbs > x.nb_symbs)? 0 : x.nb_etats); i < r.nb_etats; i++){
        r.trans[i].resize(r.nb_symbs);
    }

    for(size_t i=0; i < y.nb_etats; i++){
        for(size_t symb=0; symb < y.nb_symbs; symb++){ // symb représente le symbole courant
            etatset_t etatArr; // temporaire permetant de changer les numéro des états d'arrivée de y.trans[i][j]
            for(etatset_t::iterator it = y.trans[i][symb].begin(); it != y.trans[i][symb].end(); it++){
                etatArr.insert(*it + x.nb_etats); // x.nb_etat nous sert d'offset pour renuméroter les états
            }
            r.trans[i+x.nb_etats][symb] =  etatArr;
        }
        etatset_t etatEpsArr; // temporaire permetant de changer les numéros des états d'arrivée de y.epsilon[i]
        for(etatset_t::iterator it = y.epsilon[i].begin(); it != y.epsilon[i].end(); it++){
            etatEpsArr.insert(*it + x.nb_etats);
        }
        r.epsilon[i+x.nb_etats] = etatEpsArr;
    }

    return r;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Union(const sAutoNDE& x, const sAutoNDE& y){
    sAutoNDE r = Append(x, y);

    r.nb_etats += 1;
    r.trans.resize(r.nb_etats); // redimension au cas ou il y a réutilisation après
    r.trans[r.nb_etats-1].resize(r.nb_symbs);
    r.epsilon.resize(r.nb_etats);
    r.epsilon[r.nb_etats-1].insert(x.initial);
    r.epsilon[r.nb_etats-1].insert(y.initial+x.nb_etats);
    r.initial = r.nb_etats-1;

    return r;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Concat(const sAutoNDE& x, const sAutoNDE& y){
    sAutoNDE r = Append(x, y);
    r.nb_finaux = y.nb_finaux;

    for(etatset_t::iterator it = x.finaux.begin(); it != x.finaux.end(); it++){
        r.epsilon[*it].insert(y.initial + x.nb_etats);
        r.finaux.erase(*it); // on supprime les état fianaux de x
    }

    return r;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Complement(const sAutoNDE& x){
    sAutoNDE r;

    if(!EstDeterministe(x)){
        r = Determinize(x); // il faut le déterminiser pour un complément
    }
    else{
        r = x;
    }
    etatset_t cpFinaux = r.finaux;
    r.finaux.clear();
    for(size_t i=0; i < r.nb_etats; i++){ // inverse les finiaux et les non finaux
        if(cpFinaux.find(i) == cpFinaux.end()){
            r.finaux.insert(i);
        }
    }
    r.nb_finaux = r.finaux.size();

    return r;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Kleene(const sAutoNDE& x){
    sAutoNDE r = x;

    r.nb_etats += 1; // ajoute un nouvel état devant l'automate
    r.initial = r.nb_etats-1;
    r.trans.resize(r.nb_etats); // redimension au cas ou il y a réutilisation après
    r.trans[r.initial].resize(r.nb_symbs);
    r.epsilon.resize(r.nb_etats);
    r.epsilon[r.initial].insert(x.initial);
    r.finaux.insert(r.nb_etats-1);
	r.nb_finaux = r.finaux.size();

    // ajoute les transitions permetant de revenir des états finaux à l'état initial de x
    for(etatset_t::iterator it = x.finaux.begin(); it != x.finaux.end(); it++){
        r.epsilon[*it].insert(x.initial);
    }

    return r;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Intersection(const sAutoNDE& x, const sAutoNDE& y){
    sAutoNDE r;
    r = Union(Complement(x), Complement(y));
    return Complement(r); // la loi de Morgan : r = non(nonX U nonY)
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Produit(const sAutoNDE& x, const sAutoNDE& y){
    sAutoNDE r;
    r.nb_etats = x.nb_etats * y.nb_etats;
    r.nb_symbs = ((x.nb_symbs > y.nb_symbs)? y.nb_symbs : x.nb_symbs);
    map<pair<etat_t, etat_t>, etat_t> listNwStates; // les nouveaux états du produit

    r.epsilon.resize(r.nb_etats);
    r.trans.resize(r.nb_etats);
    for(size_t i=0; i < r.nb_etats; i++){
        r.trans[i].resize(r.nb_symbs);
    }


    for(size_t i=0; i < y.nb_etats; i++){
        for(size_t j=0; j < x.nb_etats; j++){
            pair<map<pair<etat_t, etat_t>, etat_t>::iterator , bool> res = listNwStates.insert(pair<pair<etat_t, etat_t>, etat_t>(make_pair(j, i), listNwStates.size()));
            etat_t etatDep = res.first->second; // le numéro de l'état de départ
            if(res.second){ // si c'est un nouvel état
                if(x.initial == j && y.initial == i){ // etat initial, la pair contenant l'état initial de x et y
                    r.initial = etatDep;
                }
                if(x.finaux.find(j) != x.finaux.end()){ // regarde si l'état de gauche est final dans x
                    r.finaux.insert(etatDep);
                }
            }

            //----------------- Transitions simples  ----------------------
            for(size_t symb=0; symb < r.nb_symbs; symb++){
                if(!x.trans[j][symb].empty() && !y.trans[i][symb].empty()){
                    for(etatset_t::iterator itX = x.trans[j][symb].begin(); itX != x.trans[j][symb].end(); itX++){
                        for(etatset_t::iterator itY = y.trans[i][symb].begin(); itY != y.trans[i][symb].end(); itY++){
                            res = listNwStates.insert(pair<pair<etat_t, etat_t>, etat_t>(make_pair(*itX, *itY), listNwStates.size()));
                            etat_t etatArr = res.first->second; // res.first->second représente le numéro de l'état d'arrivée
                            r.trans[etatDep][symb].insert(etatArr);
                            if(res.second){ // si c'est un nouvel état
                                if(x.initial == *itX && y.initial == *itY){ // etat initial, la pair contenant l'état initial de x et y
                                    r.initial = etatArr;
                                }
                                if(y.finaux.find(*itY) != y.finaux.end()){ // on regarde si l'état de droite est final dans y
                                    r.finaux.insert(etatArr);
                                }
                            }
                        }
                    }
                }
            }

            //----------------- Epsilon fermetures ----------------------
            if(!x.epsilon[j].empty() && !y.epsilon[i].empty()){
                for(etatset_t::iterator itX = x.epsilon[j].begin(); itX != x.epsilon[j].end(); itX++){
					for(etatset_t::iterator itY = y.epsilon[i].begin(); itY != y.epsilon[i].end(); itY++){
						res = listNwStates.insert(pair<pair<etat_t, etat_t>, etat_t>(make_pair(*itX, *itY), listNwStates.size()));
						etat_t etatArr = res.first->second; // res.first->second représente le numéro de l'état d'arrivée
						r.epsilon[etatDep].insert(etatArr);
						if(res.second){ // si c'est un nouvel état
							if(x.initial == *itX && y.initial == *itY){ // etat initial, la pair contenant l'état initial de x et y
								r.initial = etatArr;
							}
							if(y.finaux.find(*itY) != y.finaux.end()){ // on regarde si l'état de droite est final dans y
								r.finaux.insert(etatArr);
							}
						}
					}
				}
			}
        }
    }

    r.nb_finaux = r.finaux.size();
    cout << "Produit : " << endl;
    cout << "Nouveau etats (" << listNwStates.size() << ")" << endl;
    for(map<pair<etat_t, etat_t>, etat_t>::iterator it = listNwStates.begin(); it != listNwStates.end(); it++){
        cout << "{" << it->first.first << "," << it->first.second << "}  -> " << it->second << endl;
    }

    return r;
}

////////////////////////////////////////////////////////////////////////////////

bool PseudoEquivalent(const sAutoNDE& a1, const sAutoNDE& a2, unsigned int word_size_max) {
    unsigned int nbSymb = ((a1.nb_symbs > a2.nb_symbs)? a1.nb_symbs : a2.nb_symbs);
    string word = "";
    cout << "Generation des mots de longueur 0... ";
    for(unsigned int i=0; word.size() <= word_size_max; i++){
        bool res1 = Accept(a1, word);
        bool res2 = Accept(a2, word);

        if(res1 != res2){ // si un des 2 automates accèpte et l'autre non (non pseudo equivalent)
            cout << "Le mot \"" << word << "\" " << ((res1)? "est": "n'est pas") << " accepte par le premier automate et " << ((res2)? "l'ai": "pas") << " par le second" << endl;
            return false;
        }

        int j;
        for(j=word.size()-1; j >= 0 && (word[j] - ASCII_A)+1 >= nbSymb;j--){
            word[j] = ASCII_A;
        }

        if(j < 0){
            cout << "Tous les mots de longueur " << word.size() << " sont acceptes ou refuses par les deux automates" << endl;
            if(word.size() < word_size_max){ // si dernier tour de boucle
                cout << "Generation des mots de longueur " << word.size()+1 << "... ";
            }
            word += ASCII_A;
        }
        else{
            word[j]++;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool Equivalent(const sAutoNDE& a1, const sAutoNDE& a2) {
    if(!EstDeterministe(a1) || !EstDeterministe(a2)){
        cout << "Equivalence : Attention, les automates doivent etre deterministes." << endl;
        return false;
    }

    if(a1.nb_symbs != a2.nb_symbs){
        cout << "Le nombre de symboles est different" << endl;
        return false;
    }
    if(a1.nb_etats != a2.nb_etats){
        cout << "Le nombre d'etats est different" << endl;
        return false;
    }
    if(a1.nb_finaux != a2.nb_finaux){
        cout << "Le nombre d'etats finaux est different" << endl;
        return false;
    }
    map<etat_t, etat_t> correspA1, correspA2;

    etatset_t tmpLsEtat; // liste temporaire contenant tous les états à traiter
    etatset_t tmpLsEtat2;
    tmpLsEtat.insert(a1.initial);
    correspA1.insert(pair<etat_t, etat_t>(a1.initial, a2.initial)); // correspondance entre les noms de l'automate 1 à 2
    correspA2.insert(pair<etat_t, etat_t>(a2.initial, a1.initial)); // correspondance entre les noms de l'automate 2 à 1
    cout << "Renommage des etats :" << endl;
    cout << " - etat " << a1.initial << " (initial) --> " << a2.initial << endl;
    do{
        tmpLsEtat2.clear();
        for(etatset_t::iterator it = tmpLsEtat.begin(); it != tmpLsEtat.end(); it++){
            for(unsigned int sym=0; sym < a1.nb_symbs; sym++){
                etat_t tmpEtatArrA1 = *(a1.trans[*it][sym].begin());
                etat_t tmpEtatArrA2 = *(a2.trans[correspA1.find(*it)->second][sym].begin());
                map<etat_t, etat_t>::iterator res = correspA1.find(tmpEtatArrA1);
                if(res == correspA1.end()){ // si on est jamais allé dans cet état
                    // si l'état actuel est déjà connu dans l'automate 2 alors qu'il ne l'ai pas dans l'automate 1
                    if(correspA2.insert(pair<etat_t, etat_t>(tmpEtatArrA2, tmpEtatArrA1)).second == false){
                        cout << "Automate 1 : De l'etat " << res->second << " par '" << (char)(ASCII_A+sym) << "' ne correspond pas a l'automate 2." << endl;
                        return false;
                    }
                    cout << " - etat " << tmpEtatArrA1 << " --> " << tmpEtatArrA2 << endl;
                    correspA1.insert(pair<etat_t, etat_t>(tmpEtatArrA1, tmpEtatArrA2)); // Insert les nouveau états connus
                    tmpLsEtat2.insert(tmpEtatArrA1);
                }
                else{
                    if(res->second != tmpEtatArrA2){ // si les états ne correspondent pas
                        cout << "Automate 1 : De l'etat " << res->second << " par '" << (char)(ASCII_A+sym) << "' ne correspond pas a l'automate 2." << endl;
                        return false;
                    }
                }
            }
            // compare les états finaux
            if((a1.finaux.find(*it) != a1.finaux.end()) != (a2.finaux.find(correspA1.find(*it)->second) != a2.finaux.end())){
                cout << "Les etats finaux de l'automate 1 et l'automate 2 ne sont pas les memes : " << *it << endl;
                return false;
            }
        }
        tmpLsEtat = tmpLsEtat2;
    }while(!tmpLsEtat.empty()); // tant qu'on à pas fait tous les états
    cout << " - Les etats finaux sont les memes" << endl;
    cout << "\t ==> Les automates sont egaux" << endl;
    return true;
}

////////////////////////////////////////////////////////////////////////////////

// On utilise ici l'algorithme de Moore
sAutoNDE Minimize(const sAutoNDE& at){
    sAutoNDE r;
    sAutoNDE nwAuto; // automate at transformé s'il est non déterministe
    vector<etatset_t> equiCl; // les classes d'équivalence

    cout << "Minimize : " << endl;
    if(!EstDeterministe(at)){ // s'il n'est pas déterministe, on le déterminise
        nwAuto = Determinize(at);
    }
    else{
        nwAuto = at;
    }
    etatset_t first;
    for(unsigned int i=0; i < nwAuto.nb_etats; i++){
        if(nwAuto.finaux.find(i) == nwAuto.finaux.end()){ // insert tous les états non finaux
            first.insert(i);
        }
    }

    if(!first.empty()){
        equiCl.push_back(first); // ajoute la classe des non finaux
    }
    equiCl.push_back(nwAuto.finaux); // ajoute la classe des finaux
    unsigned int nbEquiClPre; // Permetra de savoir s'il y a le même nombre de classe entre la liste des classes i et i-1
    int nbClAdd; // nombre de classes déjà ajoutées
    int cpt = 0; // sert juste pour l'affichage (Conpte le numéro d'équivalence qu'on fait)
    do{
        nbClAdd = 0;
        cout << "Equivalence " << cpt << " : { ";
        cpt++;
        for(unsigned int d=0; d < equiCl.size(); d++){
            cout << equiCl[d] << " ";
        }
        cout << "}" <<endl;
        nbEquiClPre = equiCl.size();
        vector<etatset_t> nwListCl; // les nouvelle classes crées à partir de la classe i
        vector<int> listSingle; // contiendra la liste des indices des classes contenant un seul état, celle si ne peuvent être équivalentes à aucun état

        for(unsigned int i=0; i < equiCl.size(); i++){
            int nbClAddForCl = 0; // nombre de classes crées dans l'équivalence i pour une classe d'équivalence i-1
            if(equiCl[i].size() > 1){
                for(etatset_t::iterator it = equiCl[i].begin(); it != equiCl[i].end(); it++){
                    bool etatAdd = false;
                    for(unsigned int j=nbClAdd; j < nwListCl.size(); j++){
                        etatset_t::iterator curState = nwListCl[j].begin(); // On regarde seulement le premier état de la liste (car tous les états de cette liste sont équivalents)
                        unsigned int symb;
                        bool equiv = true; // Reste à vrai tant que l'état courrant est équivalent à l'état auquel on le compare
                        for(symb=0; equiv && symb < nwAuto.nb_symbs; symb++){
                            etatset_t::iterator etatArr1 = nwAuto.trans[*it][symb].begin();
                            etatset_t::iterator etatArr2 = nwAuto.trans[*curState][symb].begin();
                            for(unsigned int k=0; k < equiCl.size(); k++){ // On regarde quelles sont les clases des états d'arrivée
                                // Si les 2 états d'arrivés ne sont pas dans la même classe, alors on quitte la boucle pour tester avec le premier état de la classe suivante
                                if((equiCl[k].find(*etatArr1) == equiCl[k].end()) != (equiCl[k].find(*etatArr2) == equiCl[k].end())){
                                    equiv = false;
                                    break; break;
                                }
                            }
                        }
                        if(equiv){ // si l'état appartient à cette classe (classe j)
                            nwListCl[j].insert(*it);
                            etatAdd = true;
                            break;
                        }
                    }
                    if(!etatAdd){ // ajoute une nouvelle classe
                        etatset_t nwCl;
                        nwCl.insert(*it);
                        nwListCl.push_back(nwCl);
                        nbClAddForCl++;
                    }
                }
            }
            else{
                nwListCl.push_back(equiCl[i]);
                nbClAddForCl++;
            }
            nbClAdd += nbClAddForCl;
        }
        equiCl.clear();
        for(unsigned int j=0; j < listSingle.size(); j++){
            nwListCl.push_back(equiCl[listSingle[j]]); // on rajoute les classe contenant un seul état
        }
        equiCl = nwListCl;
    }while(nbEquiClPre < equiCl.size()); // s'il y a le même nombre de classe entre l'équivalence i et i-1 alors on ne peut pas en faire plus, on quitte

    cout << "Equivalence " << cpt << " : { ";
    for(unsigned int d=0; d < equiCl.size(); d++){
        cout << equiCl[d] << " ";
    }
    cout << "}" << endl;

    r.nb_symbs = nwAuto.nb_symbs;
    r.nb_etats = equiCl.size();
    r.epsilon.resize(r.nb_etats); // utile si on fait des opérations sur cet automate par la suite
    r.trans.resize(r.nb_etats);
    for(size_t i=0; i < r.nb_etats; i++){
        r.trans[i].resize(nwAuto.nb_symbs);
    }

    for(size_t i=0; i < equiCl.size(); i++){
        etatset_t::iterator fState = equiCl[i].begin(); // fState : premier état de la classe
        for(size_t symb=0; symb < nwAuto.nb_symbs; symb++){
            etatset_t::iterator arrState = nwAuto.trans[*fState][symb].begin(); // l'état d'arrivé en fonction de l'état fState et du symbole symb
            for(size_t j=0; j < equiCl.size(); j++){
                if(equiCl[j].find(*arrState) != equiCl[j].end()){
                    r.trans[i][symb].insert(j); // i numéro état de départ, j numéro état d'arrivée
                    break;
                }
            }
        }
        // ajoute les états finaux
        for(etatset_t::iterator it = nwAuto.finaux.begin(); it != nwAuto.finaux.end(); it++){
            if(equiCl[i].find(*it) != equiCl[i].end()){
                r.finaux.insert(i);
                break;
            }
        }
        if(equiCl[i].find(nwAuto.initial) != equiCl[i].end()){
            r.initial = i;
        }
    }
    r.nb_finaux = r.finaux.size();

    return r;
}

////////////////////////////////////////////////////////////////////////////////

bool ToGraph(sAutoNDE& at, string path){
	ofstream file(path.c_str(), ios::out | ios::trunc);
	if(!file){
		return false;
	}
	file << "digraph finite_state_machine {" << endl;
	file << "\trankdir=LR;" << endl;
	file << "\tsize=\"10,10\"" << endl << endl;
	file << "\tnode [shape = doublecircle]; ";
	for(etatset_t::iterator it = at.finaux.begin(); it != at.finaux.end(); it++){
		 file << *it << " ";
	}
	file << ";"<< endl << "\tnode [shape = point ]; q;" << endl;
	file << "\tnode [shape = circle];" << endl << endl;
	file << "\tq -> " << at.initial << endl;

	for(size_t i=0; i < at.trans.size(); i++){
		for(size_t j=0; j < at.trans[i].size(); j++){
			for(etatset_t::iterator itSet = at.trans[i][j].begin(); itSet != at.trans[i][j].end(); itSet++){
				file << "\t" << i << " -> " <<  *itSet << " [label = \"" << (char)(j+ASCII_A) << "\"];" << endl;
			}
		}
	}
	file << endl;
	for(size_t i=0; i < at.epsilon.size(); i++){
		for(etatset_t::iterator itSet = at.epsilon[i].begin(); itSet != at.epsilon[i].end(); itSet++){
			file << "\t" << i << " -> " <<  *itSet << " [label = \"ε\"];" << endl;
		}
	}
	file << "}";
	file.close();

	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool ToJflap(sAutoNDE& at, string path){
	const int decalage = 100;
    TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "no" );
	doc.LinkEndChild(decl);

	TiXmlElement * elStructure = new TiXmlElement("structure");
	doc.LinkEndChild(elStructure);
    TiXmlElement * elType = new TiXmlElement("type");
    elStructure->LinkEndChild(elType);

	TiXmlText * txtType = new TiXmlText("fa"); // met "fa" dans le type
	elType->LinkEndChild(txtType);

    TiXmlElement * elAutomaton = new TiXmlElement("automaton");
    elStructure->LinkEndChild(elAutomaton);

    // ajoute les états
    for(size_t i=0; i < at.nb_etats; i++){
		ostringstream x, y;
		x << ((i + 1) / 2) * decalage;
		y << (i / 2) * decalage;
        TiXmlElement * elState = new TiXmlElement("state");
        stringstream intToStr;
        intToStr << "q" << i;
        elState->SetAttribute("id", i);
        elState->SetAttribute("name", intToStr.str().c_str());
        elAutomaton->LinkEndChild(elState);
        TiXmlElement * elX = new TiXmlElement("x");
        elState->LinkEndChild(elX);
        TiXmlText * txtX = new TiXmlText(x.str().c_str());
        elX->LinkEndChild(txtX);
        TiXmlElement * elY = new TiXmlElement("y");
        elState->LinkEndChild(elY);
        TiXmlText * txtY = new TiXmlText(y.str().c_str());
        elY->LinkEndChild(txtY);
        if(i == at.initial){
            TiXmlElement * elInitial = new TiXmlElement("initial");
            elState->LinkEndChild(elInitial);
        }
        if(at.finaux.find(i) != at.finaux.end()){
            TiXmlElement * elFinal = new TiXmlElement("final");
            elState->LinkEndChild(elFinal);
        }
    }

    // ajoute les transitions
    for(size_t i=0; i < at.nb_etats; i++){
        for(size_t symb=0; symb < at.nb_symbs; symb++){
            if(!at.trans[i][symb].empty()){
                for(etatset_t::iterator it = at.trans[i][symb].begin(); it != at.trans[i][symb].end(); it++){
                    stringstream intToStr;
                    TiXmlElement * elTransition = new TiXmlElement("transition");
                    elAutomaton->LinkEndChild(elTransition);
                    TiXmlElement * elFrom = new TiXmlElement("from");
                    TiXmlElement * elTo = new TiXmlElement("to");
                    TiXmlElement * elRead = new TiXmlElement("read");
                    intToStr << i;
                    TiXmlText * txtFrom = new TiXmlText(intToStr.str().c_str());
                    elFrom->LinkEndChild(txtFrom);
                    intToStr.str("");
                    intToStr << *it; // it etat d'arrivée
                    TiXmlText * txtTo = new TiXmlText(intToStr.str().c_str());
                    elTo->LinkEndChild(txtTo);
                    intToStr.str("");
                    intToStr << (char)(symb + ASCII_A);
                    TiXmlText * txtRead = new TiXmlText(intToStr.str().c_str());
                    elRead->LinkEndChild(txtRead);
                    elTransition->LinkEndChild(elFrom);
                    elTransition->LinkEndChild(elTo);
                    elTransition->LinkEndChild(elRead);
                }
            }
        }

        // ajoute les epsilon transitions
        if(!at.epsilon[i].empty()){
            for(etatset_t::iterator it = at.epsilon[i].begin(); it != at.epsilon[i].end(); it++){
                stringstream intToStr;
                TiXmlElement * elTransition = new TiXmlElement("transition");
                elAutomaton->LinkEndChild(elTransition);
                TiXmlElement * elFrom = new TiXmlElement("from");
                TiXmlElement * elTo = new TiXmlElement("to");
                TiXmlElement * elRead = new TiXmlElement("read");
                intToStr << i;
                TiXmlText * txtFrom = new TiXmlText(intToStr.str().c_str());
                elFrom->LinkEndChild(txtFrom);
                intToStr.str("");
                intToStr << *it; // it etat d'arrivée
                TiXmlText * txtTo = new TiXmlText(intToStr.str().c_str());
                elTo->LinkEndChild(txtTo);
                intToStr.str("");
                elTransition->LinkEndChild(elFrom);
                elTransition->LinkEndChild(elTo);
                elTransition->LinkEndChild(elRead); // on ne met rien dans read
            }
        }

    }

	doc.SaveFile(path.c_str()); // libère tout seul la mémoire

    return true;
}
