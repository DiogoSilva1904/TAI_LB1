# Finite Context Model (FCM) Usage  

## Building the FCM Executable  
To compile the FCM program, run:  
```sh
make fcm
```

## Running FCM  
To execute FCM with a sequence file, use:  
```sh
./fcm sequences/sequence2.txt -k 3 -a 0.01
```
- `../sequences/sequence2.txt` – Input sequence file  
- `-k 3` – Context size (order)  
- `-a 0.01` – Smoothing parameter  

---

# Text Generator Usage  

## Compiling the Generator  
Use the following command to compile the text generator:  
```sh
make generator
```

## Training the model 
To train the model based on a sequence, run:  
```sh
./generator train -f sequences/sequence2.txt -k 30 -a 0.1
```
- `-f input.txt` – Path to the text file for training.
- `-a 0.01` – Smoothing parameter  
- `-k 10 ` – Maximum context size (k-value). 


## Generating Text
To generate text based on an input prompt, run: 
```sh
./generator generate -k 30 -a 0.1 -p "As armas e os barões assinalados, que da ocidental" -s 200 -t 0.6
```

- `-k 30` – Context size (must match the training context size).  
- `-a 0.1` – Smoothing parameter (must match the training value).  
- `-p " As armas e os bares assinalados, Que da ocidental"` – Initial prompt.  
- `-s 500` – Output sequence length. 
- `-t 0.2` Threshold for candidate character selection (0.0-1.0). Higher values produce more deterministic output, lower values create more variety.





