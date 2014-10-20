#include <iostream>

#include "fonctions.h"
#include "structures.h"

using namespace std;

int main(int argc, char* argv[])
{
	sAutoNDE automateNDE;
	cout << FromFile(automateNDE, "Files\\automate_D_ex1.txt") << endl;

	return 0;
}




//~ 
	//~ int main(int argc, char* argv[] ){
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
//~ 
	//~ //atr =  ExpressionRationnelle2Automate(expr);
//~ 
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
//~ 
	//~ return EXIT_SUCCESS;
	//~ }
