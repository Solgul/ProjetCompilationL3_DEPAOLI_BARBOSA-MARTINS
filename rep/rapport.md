# Rapport de Projet : Analyseur Syntaxique TPC
Binôme :

Rafael BARBOSA MARTINS 

Ugo Depoli 

Groupe : 3

## 1. Introduction
L'objectif de ce projet est de réaliser un analyseur syntaxique pour le langage TPC (un sous-ensemble du C) en utilisant les outils Flex et Bison. Le programme doit être capable de vérifier la correction lexicale et syntaxique d'un fichier source et, en l'absence d'erreurs, de construire et d'afficher l'arbre syntaxique correspondant.
Une partie centrale du travail a consisté à étendre la grammaire initiale pour supporter les structures (struct), en respectant un ensemble de contraintes strictes définies dans le sujet.

## 2. Architecture du Projet

### src/ : Contient nos fichiers sources (.l, .y, tree.c, tree.h).

### bin/ : Contient l'exécutable tpcas.

### obj/ : Contient les fichiers objets et temporaires.

### test/ : Contient nos jeux de tests (good/ et syn-err/).

### rep/ : Contient ce rapport.

Le projet est compilé via un Makefile qui génère l'exécutable bin/tpcas.

## 3. Analyse Lexicale (Flex)
Pour l'analyse lexicale, nous avons complété le fichier .lex pour reconnaître les nouveaux lexèmes nécessaires au projet.


Mots-clés : Nous avons ajouté la reconnaissance du mot-clé struct , ainsi que void, int, char, if, else, while, return.


Identificateurs ://// a completer 


Commentaires : ///// a completer 

Choix particulier : ///// a completer 

## 4. Analyse Syntaxique (Bison)
Le cœur du projet réside dans le fichier .y. Nous avons défini la grammaire pour supporter les déclarations de variables, de fonctions et surtout de structures.

### 4.1. Gestion des Structures
Conformément au sujet, nous avons intégré les structures avec les contraintes suivantes :


Déclaration : Nous autorisons la déclaration de types structures globaux (avant les fonctions) et locaux (au début d'une fonction).

Restrictions : Notre grammaire interdit explicitement :

-La définition simultanée d'un type structure et d'une variable.

-Les déclarations de structures sans champs ou sans nom.

-Les déclarations imbriquées (une struct dans une struct).


Utilisation : Les variables de type structure peuvent être utilisées dans les affectations et les expressions (ex: a.b = c).

Difficulté rencontrée : ///// a completer 

### 4.2. Construction de l'Arbre Abstrait 

Chaque règle syntaxique est associée à une action sémantique qui construit un nœud via makeNode et lie ses fils via addChild.

Pour les listes (ex: liste de paramètres, liste d'instructions), nous avons ////// a compléter 

Les opérateurs sont représentés par des nœuds internes et leurs opérandes par des fils.

## 1. Gestion des Erreurs et Interface
Le programme respecte les codes de retour demandés :

0 : Analyse réussie.

1 : Erreur syntaxique ou lexicale détectée.


2 : Erreur système (options invalides, etc.) .

En cas d'erreur, nous affichons le numéro de ligne correspondant. L'option -t ou --tree permet d'afficher l'arbre sur la sortie standard.


## 6. Tests et Validation
Nous avons constitué deux jeux de tests dans le dossier test/ :

test/good/ : Contient des fichiers .tpc valides (ex : structures imbriquées, calculs complexes).


test/syn-err/ : Contient des fichiers incorrects pour vérifier que l'analyseur rejette bien les interdits (ex : typedef interdit , déclaration de struct après instruction ).


Nous avons implémenté un script de test qui lance l'analyseur sur tous les fichiers et produit un rapport de score global.

## 7. Conclusion
Ce projet nous a permis de comprendre le fonctionnement interne d'un compilateur. Nous avons réussi à implémenter l'analyseur lexicale et syntaxique demandé. [ ... Phrase de conclusion personnelle : "L'aspect le plus difficile a été la gestion des conflits shift/reduce..." ou "Nous sommes satisfaits d'avoir un parser robuste..." ... ]