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

## 3. Fonctionnalités :
L'analyseur permet de prendre un fichier en paramètre et nous afficher l'abre correspondant si ce fichier ne contient pas d'erreur.

Un fichier peut être mis en input via < ou en étant mis en argument.

Exemple : 

- ./bin/tpcas [OPTIONS] FILE.tpc 

- ./bin/tpcas [OPTIONS] < FILE.tpc

-h permet d'afficher l'help de l'analyseur syntaxique.

-t permet d'afficher l'arbre syntaxique généré.

Le programme a aussi unegestion d'erreur qui renvoie 0 si tout se passe bien.
1 en cas d'erreur syntaxique et 2 en cas d'autre type d'erreur.

## 4. Difficultés : 

- Les plus grosses difficultés on été de crée des jeux de test ce fut assez compliquer d'arrvier a un haut poucentages de jeux de test.

- Et la 2ème plus grande diffulcultées fut de généré l'arbre nous avons mis du temps a regler tout les erreur de segmentation.