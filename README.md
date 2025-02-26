# Finite Context Model (FCM) Usage  

## Building the FCM Executable  
To compile the FCM program, run:  
```sh
make fcm
```

## Running FCM  
To execute FCM with a sequence file, use:  
```sh
./fcm ../sequences/sequence2.txt -k 3 -a 0.01
```
- `../sequences/sequence2.txt` – Input sequence file  
- `-k 3` – Context size (order)  
- `-a 0.01` – Smoothing parameter  

---

# Text Generator Usage  

## Compiling the Generator  
Use the following command to compile the text generator:  
```sh
g++ generator.cpp -o generator -std=c++11
```

## Running the Generator  
To generate text based on an input prompt, run:  
```sh
./generator -k 50 -a 0.01 -p " As armas e os bares assinalados, Que da ocidental" -s 500
```
- `-k 50` – Context size  
- `-a 0.01` – Smoothing parameter  
- `-p " As armas e os bares assinalados, Que da ocidental"` – Initial prompt  
- `-s 500` – Output sequence length  



