#include <iostream>
#include <fstream>
#include <mpi.h>
#include <chrono>
#include <vector>
#include "mpi.h"

using namespace std;

string file1 = "C:\\Users\\Lenovo\\Desktop\\numbers_large.txt";
string file2 = "C:\\Users\\Lenovo\\Desktop\\results.txt";

void print_vector(int x[], int n)
{
    std::ofstream fileO(file2);
    for (int i = 0; i < n; i++)
        fileO << x[i] << " ";
    fileO.close();
}

int numere[1000005];
int result[1000005];
int impare[1000005];
int pare[1000005];
int nrPI[1000005];
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Status status;
   /* int numere[10000];
    int result[10000];*/
    int numereSize = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    const int WORKERS = size - 1; // Procesele de lucru

    int X;
    if (rank == 0) {
        cout << "Citeste X:";
        cin >> X;
        std::ifstream file(file1);
        if (!file.is_open()) {
            std::cerr << "Eroare la deschiderea fisierului!" << std::endl;
            return 1;
        }


        int numar;


        while (file >> numar) {
            numere[numereSize++] = numar;
        }

        file.close();
    }

    //Broadcast: trimite x de la root (0) catre toate procesele
    MPI_Bcast(&X, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {

        int procesePare = WORKERS / 2;
        int proceseImpare = (WORKERS + 1) / 2;

        // procese pare
        int chunk_size_pare = numereSize / procesePare;
        int chunk_rest_pare = numereSize % procesePare;

        int start = 0, end = chunk_size_pare;

        for (int i = 1; i <= WORKERS; i++) {
            if (i % 2 == 0) {
                if (chunk_rest_pare > 0) {
                    chunk_rest_pare--;
                    end++;
                }

                // int pare[10000];

                int index = 0;
                for (int i = start; i < end; i += 2, index++) {
                    pare[index] = numere[i];
                }

                //print_vector(pare, index);

                // send start and end
                MPI_Send(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(&end, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(&index, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

                // send parts of the numbers
                MPI_Send(pare, index, MPI_INT, i, 0, MPI_COMM_WORLD);

                start = end;
                if (start % 2) start++;
                end += chunk_size_pare;

            }
        }

        // procese impare
        int chunk_size_impare = numereSize / proceseImpare;
        int chunk_rest_impare = numereSize % proceseImpare;

        start = 1;
        end = chunk_size_impare;

        for (int i = 1; i <= WORKERS; i++) {
            if (i % 2) {
                if (chunk_rest_impare > 0) {
                    chunk_rest_impare--;
                    end++;
                }

                /*int impare[10000];*/

                int index = 0;
                for (int i = start; i < end; i += 2, index++) {
                    impare[index] = numere[i];
                }

                //print_vector(impare, index);

                // send start and end
                MPI_Send(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(&end, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(&index, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

                // send parts of the numbers
                MPI_Send(impare, index, MPI_INT, i, 0, MPI_COMM_WORLD);

                start = end;
                if (start % 2 == 0) start++;
                end += chunk_size_impare;
            }
        }

        int index, subX, pesteX, subXFinal = 0, pesteXFinal = 0;
        for (int i = 1; i <= WORKERS; i++) {
            MPI_Recv(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&end, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&index, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);

            // int nrPI[10000];
            MPI_Recv(nrPI, index, MPI_INT, i, 0, MPI_COMM_WORLD, &status);

            for (int i = start, idx = 0; i < end; i += 2, idx++) {
                result[i] = nrPI[idx];
            }

            MPI_Recv(&subX, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&pesteX, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);

            subXFinal += subX;
            pesteXFinal += pesteX;
        }

        cout << subXFinal << " " << pesteXFinal;
        print_vector(result, numereSize);
    }
    else {
        int index, start, end;

        MPI_Recv(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&end, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&index, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

        // int nrPI[1000];
        MPI_Recv(nrPI, index, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        int subX = 0;
        int pesteX = 0;

        for (int i = 0; i < index; i++) {

            if (nrPI[i] < X) {
                nrPI[i] += X;
                subX++;
            }
            else {
                nrPI[i] -= X;
                pesteX++;
            }
        }
        //print_vector(nrPI, index);

        // send parts of the numbers
        MPI_Send(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&end, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&index, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(nrPI, index, MPI_INT, 0, 0, MPI_COMM_WORLD);
        
        MPI_Send(&subX, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&pesteX, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

    }


    MPI_Finalize();
    return 0;
}
