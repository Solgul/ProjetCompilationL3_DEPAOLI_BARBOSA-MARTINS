#!/bin/bash
# test_deploy.sh

TPCAS="./bin/tpcas"
GOOD_DIR="test/good"
BAD_DIR="test/syn-err"
SCORE=0
TOTAL=0

echo "--- Début des tests ---"

# Tests positifs (doivent retourner 0)
for f in $GOOD_DIR/*.tpc; do
    [ -e "$f" ] || continue
    ((TOTAL++))
    $TPCAS < "$f" > /dev/null 2>&1
    RET=$?
    if [ $RET -eq 0 ]; then
        echo "[OK] $f"
        ((SCORE++))
    else
        echo "[FAIL] $f (Attendu: 0, Reçu: $RET)"
    fi
done

# Tests négatifs (doivent retourner 1)
for f in $BAD_DIR/*.tpc; do
    [ -e "$f" ] || continue
    ((TOTAL++))
    $TPCAS < "$f" > /dev/null 2>&1
    RET=$?
    if [ $RET -eq 1 ]; then
        echo "[OK] $f"
        ((SCORE++))
    else
        echo "[FAIL] $f (Attendu: 1, Reçu: $RET)"
    fi
done

echo "--- Résultat ---"
echo "Score: $SCORE / $TOTAL"