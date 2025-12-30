# Rapport de Projet : Analyseur Syntaxique TPC
Binôme :

Rafael BARBOSA MARTINS 

Ugo Depaoli 

Groupe 3

## 1. Introduction
L'objectif de ce projet est de réaliser un analyseur syntaxique pour le langage TPC (un sous-ensemble du C) en utilisant les outils Flex et Bison. Le programme doit être capable de vérifier la correction lexicale et syntaxique d'un fichier source et, en l'absence d'erreurs, de construire et d'afficher l'arbre syntaxique correspondant.
Une partie centrale du travail consiste à étendre la grammaire initiale pour supporter les structures (struct), en respectant un ensemble de contraintes strictes définies dans le sujet ainsi que créer le parser correspondant.

## 2. Architecture du Projet

### src/ : Contient nos fichiers sources (le lexer, le parser, les fichier pour faire l'arbre).

### bin/ : Contient l'exécutable "tpcas".

### obj/ : Contient les fichiers objets et temporaires.

### test/ : Contient nos jeux de tests (good/ et syn-err/).

### rep/ : Contient ce rapport.

Le projet est compilé via un Makefile qui génère l'exécutable bin/tpcas.

## 3. Fonctionnalités :
L'analyseur permet de prendre un fichier en paramètre et nous afficher l'abre correspondant si ce fichier ne contient pas d'erreur.

Un fichier peut être mis en input via < ou en étant mis en argument après les options.

Exemple : 

- ./bin/tpcas [OPTIONS] FILE.tpc 

- ./bin/tpcas [OPTIONS] < FILE.tpc

### LES OPTIONS

-h --help permet d'afficher l'aide de la ligne de commande de l'analyseur syntaxique.

-t --tree permet d'afficher l'arbre syntaxique généré.

(si un h est rentré, l'aide sera affiché et tout le reste ignoré)

## 4. Difficultés : 

- La plus grande difficultées de ce projet consistait dans la création des jeux de test, ce qui s'est avéré assez compliquer d'arrvier à atteindre un haut poucentages de jeux de test.

- Et la 2ème plus grande diffilcultées fut de généré l'arbre nous avons mis du temps a regler toutes les erreurs de type "seg fault" ou bien "syntax error" quand on faisait sur un bon fichier.
