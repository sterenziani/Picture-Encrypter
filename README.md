# TP-Cripto

- [TP-Cripto](#tp-cripto)
  - [Arguments](#arguments)
  - [To run the code](#to-run-the-code)
  - [Example runs](#example-runs)

## Arguments

- First Argument (a1):
  - Mode
  - 'd' to distribute
  - 'r' to recover
- Second Argument (a2):
  - Image file location
  - If in distribution mode, input file
  - If in recovery mode, output file
- Third Argument (a3):
  - Amount of shadows (k)
  - Integer between 4 and 6
  - Image file pixels should be divisible by k
- Fourth Argument (a4):
  - Shadow Realm (directory)
  - For all pictures in the Shadow Realm
    - The width is the same as a2's
    - The heigth is the same as a2's

## To run the code

In the project's folder:

- make all
- ./ss a1 a2 a3 a4

## Example runs

If camouflage is a valid directory with pictures of size 300x300, such as Alfred's

- ./ss d img/Alfred.bmp 4 camouflage

If the directory camouflage contains at least 5 camouflage pictures of a picture encrypted with k=5:

- ./ss r recovered.bmp 5 camouflage
