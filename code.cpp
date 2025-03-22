#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
//#include <string>

#define MAXFACTOR 10000		//Max amplitude of the signal should be 10k.
#define FRAME_SIZE 320		//Frame size is 320.
#define THRESHOLD 100000000	//It indicates the word is now uttered.
#define P 12
#define DCEND 16000

using namespace std;

long double R[P+1], frame[FRAME_SIZE], hamming[FRAME_SIZE], a[P+1][P+1], c[P+1], raisedsine[P+1];
long double w_tokhura[P] = {1.0, 3.0, 7.0, 13.0, 19.0, 22.0, 25.0, 33.0, 42.0, 50.0, 56.0, 61.0};

string vowels[5] = {"a", "e", "i", "o", "u"};

//Replace it with your path (Please change \ to \\ in path because \ will be treated as a special character)
string path = "C:\\Users\\Nihar\\Documents\\Visual Studio 2010\\Projects\\204101052_vowelRecognition\\204101052_vowelRecognition\\dataset\\";

//Helper funcion to calculate the R vector.
//Actual calculations happens here.
long double R_k(int k) {
	long double R = 0;
	for (int i = 0; i < FRAME_SIZE - k; i++) 
		R += frame[i] * frame[i+k];	
	return R;
}

void compute_vector_R() {
	for (int i = 1; i <= P; i++) 
		R[i] = R_k(i);
}

//After the completion of this function all the values of ai's will be stored at last row of our 2d array a.
void levinsondurbin() {
	long double E[P+1], k[P+1];
	E[0] = R[0];
	for (int i = 1; i <= P; i++) {
		//calculation of k's
		k[i] = R[i];
		long double temp = 0;
		for (int j = 1; j < i; j++) 
			temp += a[i-1][j] * R[i-j];
		k[i] -= temp;
		k[i] /= E[i-1];
		
		//calculation of alpha's
		a[i][i] = k[i];
		for (int j = 1; j < i; j++) 
			a[i][j] = a[i-1][j] - k[i]*a[i-1][i-j];
		
		//calculation of e's
		E[i] = (1 - k[i]*k[i]) * E[i-1];
	}
}

//Helper function for finding cepstral coefficients.
long double findcepstral(int m) {
	long double cep = 0;
	for (int k = 1; k < m; k++) 
		cep += c[k]*a[P][m-k]*k/m; 
	cep += a[P][m];
	return cep;
}

void compute_cepstral_coefficients() {
	c[0] = logl(R[0] * R[0]);
	for (int m = 1; m <= P; m++) 
		c[m] = findcepstral(m);	 
}

//stores the cepstral coefficients in the file
void store_cepstral_coefficients(long long voweltype) {
	string filename = to_string(voweltype) + ".txt";
	ofstream out;
	//we need to append each time (five frames frome each training example) hence open it in the append mode.
	out.open(filename, std::ios_base::app);
	for (int i = 1; i <= P; i++)
		out << c[i] << " ";
	out << endl;
	out.close();
}

//Function that calculates the raised sine window and sotres that in an array.
void raisedsinewindow() {
	for (int i = 1; i <= P; i++)
		raisedsine[i] = 1 + (P/2) * sin((long double)2 * acos(0.0) * i /P);
}

void apply_raised_sine_window() {
	for (int i = 1; i <= P; i++) 
		c[i] *= raisedsine[i];
}

//Function that calculates the hamming window and stores the result in an array.
void hammingwindow() {
	for (int i = 0; i < FRAME_SIZE; i++) 
		hamming[i] = 0.54 - 0.46 * cos((long double)2 * 2 * acos(0.0) * i/(FRAME_SIZE - 1));
}

void negate_ais() {
	for (int i = 1; i <= P; i++) 
		a[P][i] = -a[P][i];
}

//This function finds frame wise tokhura distance between training and testing cepstral coefficients 
long double tokhura(vector<long double>&a, vector<long double>&b) {
	long double ans = 0;
	int size = a.size();
	for (int i = 0; i < size; i++) 
		ans += w_tokhura[i] * pow(abs(a[i] - b[i]), 2);
	return ans;
}

//This function finds average tokhura distance between for all the five frames.
long double avg_tokhura(vector<vector<long double>>&a, vector<vector<long double>>&b) {
	int rows = a.size();
	long double result = 0;
	for (int i = 0; i < rows; i++)
		result += tokhura(a[i], b[i]);
	return result/rows;
}

/**
* This function does all the preprocessing in our signal like finding the DC shift value, normalization, Finding the 
  steady part of the signal and then framing of the signal
* After preprocessing the signal we find ais and cis from this function and store them in a file.
* For each vowel utterance, cis for 5 frames will be stored in the file and this file will keep getting appended for another 10 vowels.
*/
void train_test(string filename, long long voweltype) {
	ifstream file;
	file.open(filename);

	vector<long double> signal;
	long double val, maxval = 0, factor, r0 = 0, DCVAL = 0;
	long long counter = 0, skipcounter = 0, framecounter = 0;

	//Do DC shift, Find max value and store signal in the vector in same loop.
	while (!file.eof()) {
		file >> val;

		//Skip initial 10 frames because there may be hardware problem on some device which can lead to spike of energy in the beginning
		skipcounter++;
		
		//After 10th frame till 1 second DC Value is calculated.
		//Assumption here is for first 1 second of the signal nothing is uttered.
		if (skipcounter >= 10*FRAME_SIZE && skipcounter < DCEND) {
			DCVAL += val;
			continue;
		}
		
		if (skipcounter == DCEND)
			DCVAL /= (DCEND - FRAME_SIZE);

		if (skipcounter > DCEND) {
			val -= DCVAL;	
			maxval = max(abs(val), maxval);
			signal.push_back(val);
		}
	}

	//maxfactor is predefined to 10k.
	factor = MAXFACTOR/maxval;

	//we don't require file anymore.
	file.close();
	
	//Calculate the hamming window and raised sine window.
	hammingwindow();
	raisedsinewindow();

	//Normalization + calculation of ai's and ci's.
	int size = signal.size();
	for (int i = 0; i < size; i++) {
		signal[i] *= factor * hamming[counter];
		r0 += signal[i] * signal[i];
		frame[counter] = signal[i];
		counter++;

		//we need only five frames.
		if (framecounter == 5)
			break;

		if (counter == FRAME_SIZE) {
			if (r0 > THRESHOLD) {
				framecounter++;
				R[0] = r0;
				compute_vector_R();
				levinsondurbin();
				//negate_ais();
				compute_cepstral_coefficients();
				apply_raised_sine_window();
				store_cepstral_coefficients(voweltype);
			}
			counter = 0, r0 = 0;
		}
	}
}

//FUNCTIONS RELATED TO FILES TO MAKE PROGRAM READABLE.
void average_cis() {
	int row = 0, col = 0;
	for (long long i = 0; i < 5; i++) {
		string fname = to_string(i) + ".txt";
		ifstream dataset(fname);
		string line;
		vector<vector<long double>> data(5, vector<long double>(12));
		while (getline(dataset, line)) {
			stringstream lineStream(line);
			long double val;
			while (lineStream >> val) 
				data[row][col++] += val;

			col = 0;
			row++;
			row %= 5;
		}
		int m = data.size(), n = data[0].size();
		string finale = vowels[i] + "_dataset.txt";
		ofstream out(finale);
		for (int j = 0; j < m; j++) {
			for (int k = 0; k < n; k++) {
				long double avg = data[j][k] / 10;
				out << avg << " ";
			}
			out << endl;
		}
		out.close();
	}
}

void remove_files() {
	char arr[6];
	for (long long i = 0; i < 5; i++) {
		string fname = to_string(i) + ".txt";
		strcpy_s(arr, fname.c_str());
		remove(arr);
	}
}

 vector<vector<long double>> convert_file_to_2d_vector(string filename) {
	int row = 0, col = 0;
	ifstream dataset(filename);
	string line;
	vector<vector<long double>> a(5, vector<long double>(12));
	while (getline(dataset, line)) {
		stringstream lineStream(line);
		long double val;
		while (lineStream >> val) 
			a[row][col++] = val;				
		row++;
		col = 0;
	}
	dataset.close();
	return a;
}

int test(string fname, long long i) {
	vector<vector<long double>> curr_data = convert_file_to_2d_vector(fname);
	long double final_tokhuras[5];
	for (int k = 0; k < 5; k++) {
		string temp = vowels[k] + "_dataset.txt";
		vector<vector<long double>> vowel_k = convert_file_to_2d_vector(temp);
		long double result = avg_tokhura(curr_data, vowel_k);
		final_tokhuras[k] = result;
	}
	long double min_tokhura = INT_MAX;
	int min_ind = 1;
	for (int k = 0; k < 5; k++) {
		cout << "Tokhura distance with " << vowels[k] << " is: " << final_tokhuras[k] << endl;
		if (final_tokhuras[k] < min_tokhura) {
			min_tokhura = final_tokhuras[k];
			min_ind = k;
		}
	}
	cout << "vowel uttered " << vowels[i] << " vowel found " << vowels[min_ind];
	if (min_ind == i) 
		cout << " Identified Successfully\n" << endl;
	else {
		cout << " Error Identifying the vowel\n" << endl;
		return 1;
	}
	remove_files();
	return 0;
}

void test_live(string filename) {
	vector<vector<long double>> curr_data = convert_file_to_2d_vector(filename);
	long double final_tokhuras[5];
	for (int k = 0; k < 5; k++) {
		string temp = vowels[k] + "_dataset.txt";
		vector<vector<long double>> vowel_k = convert_file_to_2d_vector(temp);
		long double result = avg_tokhura(curr_data, vowel_k);
		final_tokhuras[k] = result;
	}
	long double min_tokhura = INT_MAX;
	int min_ind = 1;
	for (int k = 0; k < 5; k++) {
		cout << "Tokhura distance with " << vowels[k] << " is: " << final_tokhuras[k] << endl;
		if (final_tokhuras[k] < min_tokhura) {
			min_tokhura = final_tokhuras[k];
			min_ind = k;
		}
	}
	cout << "\nYou have uttered: " << vowels[min_ind] << "!\n"<< endl;
}

void menu(){
    cout << "\n________________Check which vowel you have uttered!________________" << endl;
	cout << "1: Check it in Real Time!" << endl;
	cout << "2: Check it using prerecorded files!" << endl;
	cout << "3: Train the Datasets " << endl;
	cout << "4: Exit!" << endl;
	cout << "Enter your choice: ";
}

int _tmain(int argc, _TCHAR* argv[])
{
	cout << "________________WARNINGS________________" << endl;
	cout << "If you are running this code for the first time, Please first Train the dataset by pressing key 3." << endl;
	cout << "If you choose option 1, Please don't speak for first 1 second. Recording window will stay on for 5 seconds so take your time!\n" << endl;
	int choice;
	do {
		menu();
		cin >> choice;
		switch (choice) {
			case 1:
				{
					char key;
					cout << "Press s to start recording and any other key to exit: ";
					cin >> key;
					if (key != 's')
						break;
					else {
						//Record yes or no for 10 seconds, The result will be stored in input_file.txt 
						system("Recording_Module.exe 4 input_file.wav input_file.txt");

						//Now find the cepstral coeff for this file
						train_test("input_file.txt", 0);
						test_live("0.txt");

						//Remove intermediate file(0.txt) generated by recording module so next time it wont cause errors.
						remove_files();			
					}
				}
				break;
			case 2:
				{
					/*
					* Test the correctness of our program by giving those files of vowel utterances which our program has never seen before. 
					*/
					int error = 0;
					cout << "________________Testing________________" << endl;
					for (long long i = 0; i < 5; i++) {
						string temp = path + "204101052_" + vowels[i] + "_";
						for (long long j = 11; j <= 20; j++) {
							string filename = temp + to_string(j) + ".txt";
							cout << "Testing file " << filename << "\n";
							train_test(filename, i);
							string fname = to_string(i) + ".txt";
							error += test(fname, i);
						}
					}
						
					cout << "\n\n--------------------RESULTS--------------------" << endl;
					cout << "Total Misclassification of vowels : " << error << endl;
					cout << "Accuracy of the Program is : " << (50 - error)*2 << "%\n\n" << endl;

				}
				break;
			case 3:
				{
					/*
					* Remove the files if already exists on the location.
					* This is because file appending is used in the code and if we don't remove previous files it will generate errors.
					*/
					remove_files();

					/*
					* Train the dataset.
					* 10 files per each vowel is used for training and 10 files are used for testing.
					*/
					cout << "________________Training Dataset________________" << endl;
					for (long long i = 0; i < 5; i++) {
						string temp = path + "204101052_" + vowels[i] + "_";
						for (long long j = 1; j <= 10; j++) {
							string filename = temp + to_string(j) + ".txt";
							cout << filename << endl;
							train_test(filename, i);
						}
					}
	
					/*
					* Get the average values of cis (i.e convert 50 rows to 5 rows).
					* For each vowel we will create a separate dataset(files) which will have average cepstral coefficients
					* Dimensionality is 5 rows(frames) and 12 columns(ci's).
					* These datasets will be used later for testing.
					* In total five datasets will be generated for five vowels.
					*/
					average_cis();

					/*
					* Remove the files again.
					* This is because now we need to test the files for that also we need intermediate files.
					*/
					remove_files();
				}
				break;
			case 4:
				break;
			default:
				cout << "Invalid Input" << endl;
				break;
		}
	
	} while (choice != 4);
	
	return 0;
}