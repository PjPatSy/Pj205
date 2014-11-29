#include "ndet.h"

// -----------------------------------------------------------------------------
// Fonctions � compl�ter pour la premi�re partie du projet
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
			getline(myfile,line);
		} while (line.empty() || line[0]=='#');
		// on autorise les lignes de commentaires : celles qui commencent par '#'
		iss.str(line);
		if((iss >> at.nb_etats).fail() || (iss >> at.nb_symbs).fail() || (iss >> at.nb_finaux).fail())
			return false;
		// la deuxi�me ligne donne l'�tat initial
		do{
			getline (myfile,line);
		} while (line.empty() || line[0]=='#');
		iss.clear();
		iss.str(line);
		if((iss >> at.initial).fail())
			return -1;

		// les autres lignes donnent les �tats finaux
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

		// on alloue les vectors � la taille connue � l'avance pour �viter les resize dynamiques
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

			// si une des trois lectures echoue, on passe � la suite
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

// ATTENTION les balises JFF doivent �tre dans le bon ordre
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
		// les jff g�n�r�s vias JFLAP ajoutent la balise automaton
		if(strcmp(elem->Value(), "automaton") == 0){
			elem = elem->FirstChildElement();
		}

		// R�cup�re les �tats
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

		set<int> listSymbs; // liste des symboles trouv�s
		// on r�cup�re les transitions
		while(elem && strcmp(elem->Value(), "transition") == 0){
			int from, to, read;
			bool epsilonTrans = false; // true si on trouve une epsilon transition

			// On admet que les balise from, to, et read sont pr�sentes
			for(TiXmlElement* tmpEl = elem->FirstChildElement(); tmpEl; tmpEl = tmpEl->NextSiblingElement()){
				if(strcmp(tmpEl->Value(), "from") == 0){
					from = atoi(tmpEl->GetText());
				}
				else if(strcmp(tmpEl->Value(), "to") == 0){
					to = atoi(tmpEl->GetText());
				}
				else if(strcmp(tmpEl->Value(), "read") == 0){
					if(tmpEl->GetText() == NULL){
						epsilonTrans = true; // aucun carct�re � lire car epsilon transition
					}
					else{
						read = tmpEl->GetText()[0]; // le premier caract car c'est le seul
						listSymbs.insert(read); // si on insert 2 fois le m�me carct, la taille ne change pas
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
    // si on trouve une esplion transition, le graphe est non d�terministe
    for(size_t i=0; i < at.epsilon.size(); i++){
        if(!at.epsilon[i].empty()){
            return false;
        }
    }

    // si on a plus de 2 �tats pour un �tat de d�part et une transition, le graphe est non d�terministe
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
    size_t numCar = c - ASCII_A; 	// num�ro du caract�re
    etatset_t D_etat; 				// liste des �tats par Delta

	// si le caract�re n'est pas dans l'alphabet, retour d'une liste d'�tat vide
    if(numCar > at.nb_etats){
        return D_etat;
    }

    for(size_t i=0; i < at.trans.size(); i++){
        if(e.find(i) != e.end() && !at.trans[i][numCar].empty()){ // l'�tat i est demand� et sa transision par c existe
            set<etat_t>::iterator itTrans;
            for(itTrans = at.trans[i][numCar].begin(); itTrans != at.trans[i][numCar].end(); itTrans++){
                D_etat.insert(*itTrans);
            }
        }
    }
	// retourne la fermeture par e-transition des �tats trouv�s
    Fermeture(at, D_etat);
    return D_etat;
}

////////////////////////////////////////////////////////////////////////////////

bool Accept(const sAutoNDE& at, string str){
    etatset_t tmp; // liste d'�tat temporaire, sera chang�e � chaque transition
    tmp.insert(at.initial);
    Fermeture(at, tmp);
    while(!str.empty()){
        tmp = Delta(at, tmp, str[0]);
        // si aucun �tat n'a pour transition la lettre i, le mot n'est pas valide
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
	// possible d'�crire cette fonction en r�cursif, mais utilise plus de temps de calcul
    // car on repart de la liste des �tats de base avec les nouveau ajouter
    // ici on traite d'abords les �tats de base et ind�pendement les nouveaux ajout�s
    etatset_t::iterator itEt, itEps;
    etatset_t tmp1 = e; // liste d'�tat temporaire, � chaque fois qu'on trouve un �tat
    do{
        etatset_t tmp2; // liste d'�tat, � chaque fois qu'on trouve un �tat par une e-transition
        for(size_t i=0; i < at.epsilon.size(); i++){
            itEt = tmp1.find(i);
            if(itEt != tmp1.end() && !at.epsilon[i].empty()){
                for(itEps = at.epsilon[i].begin(); itEps != at.epsilon[i].end(); itEps++){
                    if(e.insert(*itEps).second){
						// si l'�tat ins�r� n'est pas d�j� dans la liste d'�tats
                        tmp2.insert(*itEps);
                    }
                }
            }
        }
        // On se d�place sur les nouveaux �tat
        tmp1 = tmp2;
    }while(!tmp1.empty());
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Determinize(const sAutoNDE& at){
    sAutoNDE rAutoD; // automate d�terminis�
    cout << "Determinise : " << endl;
    if(!EstDeterministe(at)){
        map_t corespEtat; // fait la correspondance entre le num�ro d'�tat dans rAutoD, et sa les �tats qu'il repr�sente dans at
        map_t fermeture; // Pas tr�s utile, seulement pour l'affichage (Pas obligatoire)
        rAutoD.nb_symbs = at.nb_symbs;
        rAutoD.nb_finaux = 0;

          //////////////////////////////////////////////////////////////////////////////////////////
         ////                             Les epsilon Fermeture                                ////
        //////////////////////////////////////////////////////////////////////////////////////////
        int offset = 0; // utile quand 2 �tats ont la m�me femeture, on rajoute qu'un seul �tat dans l'automate final
        cout << "Epsilon-fermeture:" << endl;
        for(size_t i=0; (i+offset) < at.nb_etats; i++){
            etatset_t tmp; // contient l'�tat dont on doit calculer la fermeture
            tmp.insert(i);
            Fermeture(at, tmp);
            pair<std::map<etatset_t,etat_t>::iterator,bool> res; // permet d'avoir un retour sur la m�thode insert
            res = fermeture.insert(pair<etatset_t, etat_t>(tmp, i+offset));
            if(i+offset == at.initial){
                corespEtat.insert(pair<etatset_t, etat_t>(tmp, i+offset)); // On ajoute seulement l'�tat initial
            }
            cout << "   E(" << i << ") = " << tmp;
            // si cette fermeture existe d�j�, on ne rajoute pas d'�tat dans l'automate final
            if(!res.second){
                cout << " = E(" << res.first->second << ")"; // map_t fermeture sert seulement � cet affichage, n'est pas tr�s utile
                i--; // comme on a enregistr� aucun �tat dans la map correspondant � l'�tat i on reste sur cet indice
                offset++; // augmenter l'offset permet de passer l'�tat suivant dans at m�me si on est toujours sur le m�me �tat dans rAutoD
            }
            else{
                if(ContientFinal(at, tmp)){
                    rAutoD.finaux.insert(i); // ajoute l'�tat actuel comme final
                }
            }
            if((i+offset) == at.initial){
                // l'�tat initial de M' est la e-fermeture de l'�tat initial de M
                // si la e-fermeture de M est d�ja dans la map, l'�tat initial de M' � l'�tat d�j� dans la map
                // sinon l'�tat init de M' est le nouvel �tat ajout�, le retour de l'insert permet de r�aliser cette condition
                rAutoD.initial = res.first->second;
            }
            cout << endl;
        }

          //////////////////////////////////////////////////////////////////////////////////////////
         ////                             Delta                                                ////
        //////////////////////////////////////////////////////////////////////////////////////////
        map_t tmpMap1 = corespEtat;	// map temporaire permettant d'enregistrer les �tats d�s qu'on se d�place dans l'automate
        map_t tmpMap2;
        string str = ""; 			// contiendra des infos sur les transision, sera affich�e � la fin de la fonction

        while(!tmpMap1.empty()){
            for(map_t::iterator itM = tmpMap1.begin(); itM != tmpMap1.end(); itM++){
                for(size_t i=0; i < rAutoD.nb_symbs; i++){
                    pair<std::map<etatset_t,etat_t>::iterator,bool> res;
                    etatset_t deltaRes = Delta(at, itM->first, (char)(i+ASCII_A));
                    res = corespEtat.insert(pair<etatset_t, etat_t>(deltaRes, corespEtat.size()));
                    // on enregistre la transition et redimentionnement dynamique de la liste des transitions
                    // itM->second : num�ro de l'�tat d'o� on vient
                    // i : le caract�re de transition
                    // res.first->second : l'�tat ou l'on va
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
        rAutoD.epsilon.resize(rAutoD.nb_etats); // Prend de la place, mais simplifie s'il faut faire des opr�rations sur les epsilon apr�s
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

bool ToGraph(sAutoNDE& at, string path){
  //TODO d�finir cette fonction

  return true;
}


////////////////////////////////////////////////////////////////////////////////

bool ToJflap(sAutoNDE& at, string path){
    TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "no" );
	doc.LinkEndChild( decl );

	TiXmlElement * elStructure = new TiXmlElement("structure");
	doc.LinkEndChild(elStructure);
    TiXmlElement * elType = new TiXmlElement("type");
    elStructure->LinkEndChild(elType);

	TiXmlText * txtType = new TiXmlText("fa"); // Met "fa" dans le type....
	elType->LinkEndChild(txtType);

    TiXmlElement * elAutomaton = new TiXmlElement("automaton");
    elStructure->LinkEndChild(elAutomaton);

    // Ajoute les �tats
    for(int i=0; i < at.nb_etats; i++){
        TiXmlElement * elState = new TiXmlElement("state");
        stringstream intToStr;
        intToStr << "q" << i;
        elState->SetAttribute("id", i);
        elState->SetAttribute("name", intToStr.str().c_str());
        elAutomaton->LinkEndChild(elState);
        TiXmlElement * elX = new TiXmlElement("x");
        elState->LinkEndChild(elX);
        TiXmlText * txtX = new TiXmlText("0");
        elX->LinkEndChild(txtX);
        TiXmlElement * elY = new TiXmlElement("y");
        elState->LinkEndChild(elY);
        TiXmlText * txtY = new TiXmlText("0");
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

    // Ajoute les transitions
    for(int i=0; i < at.nb_etats; i++){
        for(int symb=0; symb < at.nb_symbs; symb++){
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
                    intToStr << *it; // it etat d'arriv�e
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

        // Ajoute les epsilon transitions
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
                intToStr << *it; // it etat d'arriv�e
                TiXmlText * txtTo = new TiXmlText(intToStr.str().c_str());
                elTo->LinkEndChild(txtTo);
                intToStr.str("");
                elTransition->LinkEndChild(elFrom);
                elTransition->LinkEndChild(elTo);
                elTransition->LinkEndChild(elRead); // On ne met rien dans read
            }
        }

    }



	doc.SaveFile(path.c_str()); // Lib�re tout seul la m�moire

    return true;
}

////////////////////////////////////////////////////////////////////////////////

// fonction outil : on garde x, et on "ajoute" trans et epsilon de y
// en renommant ses �tats, id est en d�callant les indices des �tats de y
// de x.nb_etats
sAutoNDE Append(const sAutoNDE& x, const sAutoNDE& y){
    //TODO d�finir cette fonction

    assert(x.nb_symbs == y.nb_symbs);
    sAutoNDE r = x;
    r.nb_etats += y.nb_etats;
    r.nb_finaux += y.nb_finaux;
    r.nb_symbs = ((y.nb_symbs > x.nb_symbs)? y.nb_symbs : x.nb_symbs);
    for(etatset_t::iterator it = y.finaux.begin(); it != y.finaux.end(); it++){
       r.finaux.insert(x.nb_etats + *it);
    }
    r.epsilon.resize(r.nb_etats);
    r.trans.resize(r.nb_etats);
    // Resize (i = 0 si y � plus de symboles que x et i = nombre d'�tat de x sinon, pour resize seulement les trans de y dans r
    for(int i=((r.nb_symbs > x.nb_symbs)? 0 : x.nb_etats); i < r.nb_etats; i++){
        r.trans[i].resize(r.nb_symbs);
    }

    for(int i=0; i < y.nb_etats; i++){

        for(int symb=0; symb < y.nb_symbs; symb++){ // symb repr�sente le symbole courant
            etatset_t etatArr; // Temporaire permetant de changer les num�ro des �tats d'arriv�e de y.trans[i][j]
            for(etatset_t::iterator it = y.trans[i][symb].begin(); it != y.trans[i][symb].end(); it++){
                etatArr.insert(*it + x.nb_etats); // x.nb_etat nous sert d'offset pour renum�rot� les �tats
            }
            r.trans[i+x.nb_etats][symb] =  etatArr;
        }
        etatset_t etatEpsArr; // Temporaire permetant de changer les num�ro des �tats d'arriv�e de y.epsilon[i]
        for(etatset_t::iterator it = y.epsilon[i].begin(); it != y.epsilon[i].end(); it++){
            etatEpsArr.insert(*it + x.nb_etats);
        }
        r.epsilon[i+x.nb_etats] = etatEpsArr;
    }

    return r;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Union(const sAutoNDE& x, const sAutoNDE& y){
  //TODO d�finir cette fonction

    assert(x.nb_symbs == y.nb_symbs);
    sAutoNDE r = Append(x, y);

    r.nb_etats += 1;
    r.trans.resize(r.nb_etats); // Redimension au cas ou il y a r�utilisation apr�s
    r.trans[r.nb_etats-1].resize(r.nb_symbs); // "    "     "

    r.epsilon.resize(r.nb_etats);
    r.epsilon[r.nb_etats-1].insert(x.initial);
    r.epsilon[r.nb_etats-1].insert(y.initial+x.nb_etats);
    r.initial = r.nb_etats-1;
    return r;
}

////////////////////////////////////////////////////////////////////////////////


sAutoNDE Concat(const sAutoNDE& x, const sAutoNDE& y){
    assert(x.nb_symbs == y.nb_symbs);
    sAutoNDE r = Append(x, y);
    r.nb_finaux = y.nb_finaux;


    for(etatset_t::iterator it = x.finaux.begin(); it != x.finaux.end(); it++){
        r.epsilon[*it].insert(y.initial + x.nb_etats);
        r.finaux.erase(*it); // on supprime les �tat fianaux de x
    }

    return r;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Complement(const sAutoNDE& x){
    sAutoNDE r;
    if(!EstDeterministe(x)){
        r = Determinize(x); // Il faut le d�terminiser pour un compl�ment
    }
    else{
        r = x;
    }
    etatset_t cpFinaux = r.finaux;
    r.finaux.clear();
    for(int i=0; i < r.nb_etats; i++){ // Inverse les finiaux et les non finauw
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
    r.nb_etats += 1; // Ajoute un nouvel �tat devant l'automate
    r.trans.resize(r.nb_etats); // Redimension au cas ou il y a r�utilisation apr�s
    r.trans[r.nb_etats-1].resize(r.nb_symbs); // "    "     "

    r.epsilon.resize(r.nb_etats);
    r.epsilon[r.nb_etats-1].insert(x.initial);
    r.initial = r.nb_etats-1;
    r.finaux.insert(r.nb_etats-1);

    // Ajoute les transition permetant de revenir des �tats finaux � l'�tat initial de x
    for(etatset_t::iterator it = x.finaux.begin(); it != x.finaux.end(); it++){
        r.epsilon[*it].insert(x.initial);
    }

    return r;
}

////////////////////////////////////////////////////////////////////////////////

// Intersection avec la loi de De Morgan
sAutoNDE Intersection(const sAutoNDE& x, const sAutoNDE& y){
    sAutoNDE r;
    r = Union(Complement(x), Complement(y));
    return Complement(r); // La loi de Morgan : r = non(nonX U nonY)
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
    sAutoNDE r;
    vector<etatset_t> equiCl; // Les classes d'�quivalence

    if(!EstDeterministe(at)){ // S'il n'est pas d�terministe, on le d�terminise
        r = Determinize(at);
    }
    else{
        r = at;
    }
    etatset_t first;
    for(int i=0; i < r.nb_etats; i++){
        if(r.finaux.find(i) == r.finaux.end()){ // Insert tous les �tats non finaux
            first.insert(i);
        }
    }
    if(!first.empty()){
        equiCl.push_back(first);
    }
    equiCl.push_back(at.finaux);
    int dd = 0;
    cout << "First : " << first << endl;

    while(dd != 5){
        for(int i=0; i < equiCl.size(); i++){
            vector<etatset_t> nwListCl; // Les nouvelle classes cr�es � partir de la classe i
            for(etatset_t::iterator it = equiCl[i].begin(); it != equiCl[i].end(); it++){
                etatset_t::iterator it2 = it;
                for(it2++; it2 != equiCl[i].end(); it2++){
                   //cout << "it 2 : " << *it2 << endl;
                    bool same = true;
                    for(int symb=0; same == true && symb < at.nb_symbs; symb++){
                        for(int j=0; j < equiCl.size(); j++){
                            etatset_t::iterator etatArr1 = at.trans[*it][symb].begin();
                            etatset_t::iterator etatArr2 = at.trans[*it2][symb].begin();
                            cout << "It " << *it2 << "Symb : " << (char)(symb+ASCII_A) << endl;
                            cout << "Res : " << *etatArr2 << endl;
                            equiCl[j].find(6);
//                            if(equiCl[j].find(*etatArr1) != equiCl[j].find(*etatArr2)){
//                                same = false;
//                                break;
//                            }
                        }
                    }
                }
            }
        }


        dd++;
    }

    return r;
}


////////////////////////////////////////////////////////////////////////////////

// d�termine la pseudo �quivalence par comparaison de tous les mots de Sigma* de longueur < � word_size_max
bool PseudoEquivalent(const sAutoNDE& a1, const sAutoNDE& a2, unsigned int word_size_max) {
    int nbSymb = ((a1.nb_symbs > a2.nb_symbs)? a1.nb_symbs : a2.nb_symbs);
    int curSymb = 0;
    string word = "";
    for(int i=0; i < word_size_max; i++){
        cout << "Word : " << word << endl; // ------------------- Supprimer l'affichage
        if(Accept(a1, word) != Accept(a2, word)){ // Si un des 2 automates acc�pte et l'autre non (non pseudo equivalent)
            return false;
        }

        int j;
        for(j=word.size()-1; j >= 0 && (word[j] - ASCII_A)+1 >= nbSymb;j--){
            word[j] = ASCII_A;
        }
        if(j < 0){
            word += ASCII_A;
        }
        else{
            word[j]++;
        }

    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

// d�termine l'�quivalence par "�galit�" des automates :
//   - m�me nombre d'�tats
//   - m�me �tat initial
//   - m�mes �tats finaux
//   - m�me table de transition
// � un renommage des �tats pr�s
bool Equivalent(const sAutoNDE& a1, const sAutoNDE& a2) {
//    int nbSym = ((a1.nb_symbs > a2.nb_symbs)? a1.nb_symbs : a2.nb_symbs);
//    if(a1.nb_etats != a2.nb_etats){
//        return false;
//    }
//    map<etat_t, etat_t> coresp; // Permetant d'avoir la corespondance entre le num�ro d'�tat de a1 et celui de a2
//    for(int i=0; i < a1.nb_etats; i++){
//        for(int j=0; j < nbSym; j++){
//
//        }
//    }

    return true;
}
