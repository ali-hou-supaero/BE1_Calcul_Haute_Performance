# K-means et calcul parallèle avec OpenMP

Bureau d’études de calcul haute performance réalisé dans le cadre de la formation ISAE-SUPAERO. Le code C implémente K-means et utilise OpenMP pour paralléliser l’affectation aux clusters et le calcul des centres.

## Contenu

- `src/kmeans.c` : algorithme, régions parallèles et chronométrage.
- `src/main.c` : génération de points synthétiques et exécutions avec 1 à 8 threads.
- `src/compress.c` : application à des images PNG.
- `include/` : en-têtes ; `images/` : images d’exemple.
- `Makefile` : compilation des exécutables `kmeans` et `compress`.
- `vision/0_cnns.ipynb` : notebook de réseaux convolutifs également présent dans le dépôt, distinct du programme OpenMP.

## Compilation et exploration

Le Makefile cible un environnement de type Linux avec GCC, GNU Make, OpenMP et les fichiers de développement de zlib.

```sh
make
./kmeans
```

Le programme affiche les durées et les centres calculés. Ses paramètres sont définis dans `src/main.c` ; le cas fourni génère 1,5 million de points. Ces commandes décrivent les points d’entrée du code ; elles n’ont pas été réexécutées pour cette mise à jour documentaire. Aucun gain de performance n’est annoncé sans mesure reproductible.

## Contexte et attribution

L’énoncé est conservé dans `03_OpenMP_BE.pdf`. Ce dépôt comprend du matériel pédagogique et des composants tiers, notamment les fichiers `spng` : conserver leurs mentions d’origine lors de toute réutilisation.
