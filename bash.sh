#!/bin/bash

make

rm -f rapport.txt

# Vérifiez si le compilateur existe et est exécutable
if [ ! -x bin/tpcc ]; then
    echo "Error: 'bin/tpcc' not found or not executable."
    exit 1
fi

# Parcourir les sous-dossiers de test
for DIR in $(ls test/); do
    echo "Folder : $DIR" >> "./rapport.txt"
    total=0
    err_syn=0
    err_sem=0
    warnings=0
    no_err=0
    for FILES in $(ls test/$DIR/); do
        # Exécuter le compilateur et capturer le code de retour
        bin/./tpcc < "test/$DIR/$FILES" 2>> /dev/null
        res=$?
        total=$((total + 1))
        
        # Ajouter un diagnostic pour le code de retour
        echo "File: $FILES, Return Code: $res" >> "./rapport.txt"
        
        case $res in
            0)
                no_err=$((no_err + 1))
                ;;
            1)
                err_syn=$((err_syn + 1))
                ;;
            2)
                err_sem=$((err_sem + 1))
                ;;
            3)
                warnings=$((warnings + 1))
                ;;
            *)
                echo "Unexpected return code $res for file $FILES in directory $DIR" >> "./rapport.txt"
                ;;
        esac
    done

    # Calcul des pourcentages
    err_syn_pct=$(( (err_syn * 100) / total ))
    err_sem_pct=$(( (err_sem * 100) / total ))
    warnings_pct=$(( (warnings * 100) / total ))
    no_err_pct=$(( (no_err * 100) / total ))

    # Écrire les résultats dans le rapport
    echo "Total files = $total, $err_syn_pct% syntax errors, $err_sem_pct% semantic errors, $warnings_pct% warnings, $no_err_pct% success" >> "./rapport.txt"
done
