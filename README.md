### Automatic Voice Recognition System

# About this Project
This code is a C++ program designed for vowel recognition using signal processing techniques. It processes audio signals to identify which vowel is being spoken. The program can be used for training a dataset, testing the accuracy of the recognition system, and performing real-time vowel recognition.

## Key Components of the Code
# Signal Processing:

DC Shift and Normalization: The code adjusts the signal to remove any DC offset and normalizes the signal amplitude.

Framing: The signal is divided into frames of size 320 samples.

Hamming Window: A Hamming window is applied to each frame to reduce spectral leakage.

Autocorrelation: The autocorrelation of the signal is computed to analyze its periodicity.

Levinson-Durbin Algorithm: This algorithm is used to compute the Linear Predictive Coding (LPC) coefficients, which are essential for speech analysis.

Cepstral Coefficients: The code computes cepstral coefficients, which are used to represent the short-term power spectrum of the signal.

# Training and Testing:

Training: The program trains on a dataset of vowel sounds. It processes multiple audio files for each vowel, computes the cepstral coefficients, and stores them.

Testing: The program tests the recognition system using a separate set of audio files. It computes the Tokhura distance between the test data and the trained data to identify the vowel.

Real-Time Recognition:

The program can also perform real-time vowel recognition by recording audio input, processing it, and comparing it against the trained dataset.

# File Handling:

The program reads and writes data to files for training and testing purposes. It uses file I/O operations to store and retrieve cepstral coefficients and other data.


# Algorithm Implementation:

The implementation of the Levinson-Durbin algorithm and the computation of cepstral coefficients are evidence of advanced signal processing techniques being used.

# Real-Time Processing:

The ability to perform real-time vowel recognition is a significant feature. It shows that the system can process audio input on-the-fly, which is essential for real-world applications.

# Accuracy and Testing:

The code includes a testing phase where the accuracy of the vowel recognition system is evaluated. The results of these tests can be used as evidence of the system's reliability and performance.

# Documentation and Readability:

The use of helper functions and modular design demonstrates good programming practices, which can be highlighted in technical documentation.

# Potential Applications:

The code can be adapted for various applications, such as voice assistants, automated transcription services, and language learning tools.
