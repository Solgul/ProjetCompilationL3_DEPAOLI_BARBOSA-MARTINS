#!/bin/bash

# Couleurs pour la lecture
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

EXEC="./bin/tpcas"
TOTAL=0
PASSED=0

# Vérification de l'existence de l'exécutable
if [ ! -f "$EXEC" ]; then
    echo -e "${RED}Erreur: L'exécutable $EXEC est introuvable. Lancez 'make' d'abord.${NC}"
    exit 2
fi

echo "--- Début des tests du projet TPC ---"

# 1. Test des fichiers VALIDES (doivent renvoyer 0)
echo -e "\n[+] Test des fichiers syntaxiquement corrects (Attendu: 0)"
for f in test/good/*.tpc; do
    [ -e "$f" ] || continue
    ((TOTAL++))
    $EXEC < "$f" > /dev/null 2>&1
    RET=$?
    if [ $RET -eq 0 ]; then
        echo -e "  $f : ${GREEN}PASS${NC}"
        ((PASSED++))
    else
        echo -e "  $f : ${RED}FAIL (Code $RET au lieu de 0)${NC}"
    fi
done

# 2. Test des fichiers INVALIDES (doivent renvoyer 1)
echo -e "\n[-] Test des fichiers avec erreurs (Attendu: 1)"
for f in test/syn-err/*.tpc; do
    [ -e "$f" ] || continue
    ((TOTAL++))
    $EXEC < "$f" > /dev/null 2>&1
    RET=$?
    if [ $RET -eq 1 ]; then
        echo -e "  $f : ${GREEN}PASS${NC}"
        ((PASSED++))
    else
        echo -e "  $f : ${RED}FAIL (Code $RET au lieu de 1)${NC}"
    fi
done

# Résultat final
echo -e "\n--------------------------------------"
echo -e "SCORE FINAL: $PASSED / $TOTAL"
echo -e "--------------------------------------"

if [ $PASSED -eq $TOTAL ]; then
    exit 0
else
    exit 1
fi
