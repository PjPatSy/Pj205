#include "ndet.h"

void test();
void Help(ostream& out, char *s);

int main(int argc, char* argv[] ){
    sAutoNDE a, b;
    sAutoNDE r;

    if(!FromFileTxt(a, "exemples/automate_ND_ex1.txt")){
        cout << "faux" << endl;
    }
    if(!FromFileTxt(b, "exemples/automate_D_ex2.txt")){
        cout << "faux 2" << endl;
    }

    cout << "A : " << a << endl;
    //cout << "B : " << b << endl;
    //r = Determinize(a);
    //r = Append(a, b);
    //r = Union(a, b);
    //r = Concat(a,b);
    //r = Complement(b);
    //r = Kleene(b);
    //r = Intersection(a, b);
//    if(PseudoEquivalent(r,a, 1000)){
//        cout << "Ils sont equivalents..." << endl;
//    }
//    else{
//        cout << "Non equivalents..." << endl;
//    }
    r = Minimize(a);
    ToJflap(a, "res.jff");
    //cout << r << endl;

    //test();
//    if(argc < 3){
//        Help(cout, argv[0]);
//        return EXIT_FAILURE;
//    }
//
//  int pos;
//  int act=-1;                 // pos et act pour savoir quelle action effectuer
//  int nb_files = 0;           // nombre de fichiers en entrée
//  string str, in1, in2, out, acc, expr;
//  unsigned int word_size_max; // pour la pseudo équivalence
//  // chaines pour (resp.) tampon; fichier d'entrée Input1; fichier d'entrée Input2;
//  // fichier de sortie et chaine dont l'acceptation est à tester
//  bool toFile=false, graphMode=false, jflapMode=false;     // sortie STDOUT ou fichier ? Si fichier, format graphviz ? Jflap ?
//
//  // options acceptées
//  const size_t NBOPT = 16;
//  string aLN[] = {"accept", "determinize", "union", "concat", "kleene", "complement", "intersection", "produit", "expressionrationnelle2automate", "minimisation", "pseudo_equivalent", "equivalent", "no_operation", "output", "graph", "jflap"};
//  string aSN[] = {"acc", "det", "cup", "cat", "star", "bar", "cap", "prod", "expr2aut", "min", "pequ", "equ", "nop", "o", "g", "j"};
//
//  // on essaie de "parser" chaque option de la ligne de commande
//  for(int i=1; i<argc; ++i){
//    if (DEBUG) cerr << "argv[" << i << "] = '" << argv[i] << "'" << endl;
//    str = argv[i];
//    pos = -1;
//    string* pL = find(aLN, aLN+NBOPT, str.substr(1));
//    string* pS = find(aSN, aSN+NBOPT, str.substr(1));
//
//    if(pL!=aLN+NBOPT)
//      pos = pL - aLN;
//    if(pS!=aSN+NBOPT)
//      pos = pS - aSN;
//
//    if(pos != -1){
//      // (pos != -1) <=> on a trouvé une option longue ou courte
//      if (DEBUG) cerr << "Key found (" << pos << ") : " << str << endl;
//      switch (pos) {
//        case 0: //acc
//          in1 = argv[++i];
//          acc = argv[++i];
//	  nb_files = 1;
//          break;
//        case 1: //det
//          in1 = argv[++i];
//	  nb_files = 1;
//          break;
//        case 2: //cup
//          in1 = argv[++i];
//          in2 = argv[++i];
//	  nb_files = 2;
//          break;
//        case 3: //cat
//          in1 = argv[++i];
//          in2 = argv[++i];
//	  nb_files = 2;
//          break;
//        case 4: //star
//          in1 = argv[++i];
//	  nb_files = 1;
//          break;
//        case 5: //bar
//          in1 = argv[++i];
//	  nb_files = 1;
//          break;
//        case 6: //cap
//          in1 = argv[++i];
//          in2 = argv[++i];
//	  nb_files = 2;
//          break;
//        case 7: //prod
//          in1 = argv[++i];
//          in2 = argv[++i];
//	  nb_files = 2;
//          break;
//        case 8: //expr2aut
//          expr = argv[++i];
//	  nb_files = 0;
//          break;
//        case 9: //min
//          in1 = argv[++i];
//	  nb_files = 1;
//          break;
//        case 10: //pequ
//          in1 = argv[++i];
//          in2 = argv[++i];
//	  word_size_max = atoi(argv[++i]);
//	  nb_files = 2;
//          break;
//        case 11: //equ
//          in1 = argv[++i];
//          in2 = argv[++i];
//	  nb_files = 2;
//          break;
//        case 12: //nop
//          in1 = argv[++i];
//	  nb_files = 1;
//          break;
//        case 13: //o
//          toFile = true;
//          out = argv[++i];
//          break;
//        case 14: //g
//          graphMode = true;
//          break;
//        case 15: //j
//          jflapMode = true;
//          break;
//        default:
//          return EXIT_FAILURE;
//        }
//    }
//    else{
//      cerr << "Option inconnue "<< str << endl;
//      return EXIT_FAILURE;
//    }
//
//    if(pos<13){
//      if(act > -1){
//        cerr << "Plusieurs actions spécififées"<< endl;
//        return EXIT_FAILURE;
//      }
//      else
//        act = pos;
//    }
//  }
//
//  if (act == -1){
//    cerr << "Pas d'action spécififée"<< endl;
//    return EXIT_FAILURE;
//  }
//
///* Les options sont OK, on va essayer de lire le(s) automate(s) at1 (et at2)
//et effectuer l'action spécifiée. Atr stockera le résultat*/
//
//  sAutoNDE at1, at2, atr;
//
//  if ((nb_files == 1 or nb_files == 2) and !FromFile(at1, in1)){
//    cerr << "Erreur de lecture " << in1 << endl;
//    return EXIT_FAILURE;
//  }
//  if (nb_files ==2 and !FromFile(at2, in2)){
//    cerr << "Erreur de lecture " << in2 << endl;
//    return EXIT_FAILURE;
//  }
//
//  switch(act) {
//  case 0: //acc
//    cout << "'" << acc << "' est accepté: " << Accept(at1, acc) << endl;
//    atr = at1;
//    break;
//  case 1: //det
//    atr = Determinize(at1);
//    break;
//  case 2: //cup
//    atr =  Union(at1, at2);
//    break;
//  case 3: //cat
//    atr =  Concat(at1, at2);
//    break;
//  case 4: //star
//    atr =  Kleene(at1);
//    break;
//  case 5: //bar
//    atr =  Complement(at1);
//    break;
//  case 6: //cap
//    atr =  Intersection(at1, at2);
//    break;
//  case 7: //prod
//    atr =  Produit(at1, at2);
//    break;
//  case 8: //expr2aut
///*
//    atr =  ExpressionRationnelle2Automate(expr);
//*/
//    break;
//  case 9: //minimisation
//    atr =  Minimize(at1);
//    break;
//  case 10: //pseudo équivalence
//    cout << "Après comparaison de tous les mots de longeur < à " << word_size_max << ", les deux automates sont pseudo-équivalents : ";
//    cout << PseudoEquivalent(at1,at2,word_size_max) << endl;
//    atr = at1;
//    break;
//  case 11: //équivalence
//    cout << "Les deux automates sont équivalents : " << Equivalent(at1,at2) << endl;
//    atr = at1;
//    break;
//  case 12: //nop
//    atr = at1;
//    break;
//  default:
//    return EXIT_FAILURE;
//  }
//
//  // on affiche le résultat ou on l'écrit dans un fichier
//  if(!toFile)
//    cout << atr;
//  else{
//    if(graphMode){
//      ToGraph(atr, out + ".gv");
//      system(("dot -Tpng " + out + ".gv -o " + out + ".png").c_str());
//    }
//    if(jflapMode){
//      ToJflap(atr, out + ".jff");
//    }
//    ofstream f((out + ".txt").c_str(), ios::trunc);
//    if(f.fail())
//      return EXIT_FAILURE;
//    f << atr;
//  }

  return EXIT_SUCCESS;
}


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
  out << "-pequ ou -pseudo_equivalent Input1 Input2 size :\n\t équivalence d'automates par comparaison mot par mot de longueur < à size" << endl;
  out << "-equ ou -equivalent Input1 Input2 :\n\t équivalence d'automates par minimisation et comparaison des tables de transition" << endl;
  out << "-nop ou -no_operation Input :\n\t ne rien faire de particulier" << endl;

  out << "-o ou -output Output :\n\t écrire le résultat dans le fichier Output, afficher sur STDOUT si non spécifié" << endl;
  out << "-g ou -graphe :\n\t l'output est au format dot/graphiz" << endl  << endl;

  out << "Exemple '" << s << " -determinize auto.txt -output determin -g'" << endl;
  out << "Exemple '" << s << " -minimisation test.jff -output min -j'" << endl;
}

// appelé lorsque 'test' est passé en paramètre au programme
void test(){
	int nbAutomate = 19;
	string listeAutomate[] = {"automate_D_ex1","automate_D_ex2","automate_NDE_ex1","automate_NDE_ex2","automate_NDE_ex3","automate_NDE_ex4","automate_NDE_ex5","automate_NDE_ex6","automate_ND_ex1","automate_ND_ex2","automate_ND_ex3","automate_ND_ex4","automate_ND_ex5","automate_ND_ex6","automate_ND_ex7","automate_ND_ex8","output1","output2","output3"};

	for (int i = 0; i < nbAutomate; i++){
		sAutoNDE automateTXT, automateJFF;

		FromFile(automateTXT, "exemples/" + listeAutomate[i] + ".txt");
		FromFile(automateJFF, "exemples/" + listeAutomate[i] + ".jff");

		system("clear");
		cout << endl << "##########   " << listeAutomate[i] << "   ##########" << endl;
		cout << "==> TXT" << automateTXT << endl;
        cout << "==> JFF" << automateJFF << endl;

		etatset_t cf;
		cf.insert(1);
		cf.insert(0);
		cout << "==> ContientFinal " << cf << " : " << ((ContientFinal(automateTXT, cf))? "Oui" : "Non") << endl;

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
