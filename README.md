# libSAN
C++/Scala library for **S**hort and Sparse **A**SCII-encoded **N**umbers.

## Motivation
It is sometimes necessary to store or transmit values in an environment that is **not 8-bit clean**.
If you then also want to be able to print the values, you are even more restricted.
To avoid encoding issues, only 7-bit ASCII is available, which includes at least 32 characters that are either not printable or you want to avoid.
This leaves 96 characters, of which many would need to be escaped, depending on the context.

While it would be possible to use an arbitrary amount of "safe" characters, it is desirable to stick to powers of 2, so the encoding/decoding is easier.
This leaves three common options:
1. Using **decimal** encoding, which is natural for humans to read/write, but only uses 10 characters, i.e., roughly 3.3 bits, and it is not a power of 2.
2. Using **hexadecimal** encoding, which is a good balance in human readability, but still only uses 4 bit.
3. Using **base64** encoding, which is no longer readable for humans, but quite efficient at 6 bits out of 8.

Base64 is the obvious choice for efficiency, but it has two unfortunate properties:
1. Since it can encode blocks of 6 bits only, which is not a proper fraction of 8 bits, the common solution is to encode 3 bytes (i.e. 24 bits) into 4 blocks of six bits, i.e., 4 characters (or 4 bytes). If the (remaining) input is shorter than 24 bits, the input is **padded** (which actually needs a 65th encoding character).
2. It does not have any support for **sparseness**, as usual in the encoding of numbers, by omitting leading zeros. In contrast to decimal and hexadecimal, which are often used for fixed precision numbers, the main application of base64 is arbitrary length binary data, where the padding is almost irrelevant.

## Description

This library
* offers a **base64-inspired** encoding for numbers, 
* which demands the user to **know the precision** of the input (e.g. 24-bit, 32-bit, 48-bit, 64-bit or 128-bit),
* but then **offers sparseness**.

The primary goal is to
* use the **fewest amount of characters** possible for input values, especially **if the numbers are quite small** and
* offer a **more efficient encoding for common bit ranges**, that are usually printed in hex.

## Examples

The default character set we use is ```0-9```, ```a-z```, ```A-Z``` and ```+```/```-```.

### 32-Bit Values (e.g. IP Addresses)

IP addresses are usually printed using decimal notation and dots, which means they use 7-15 characters.
In hexadecimal encoding without separators, they would always use 8 characters (32 bits divided by 4 bits).
In our base64 variant, they use 1-6 characters (32 bits divided by 6 bits), where the first character of six byte strings is always either ```+```, ```-```, ```0``` or ```1``` (by careful choice of the encoding table layout).

```
0.0.0.0          ->       +
255.255.255.255  ->       -
192.168.0.0      ->     az+
192.168.41.0     ->    aqz+
192.168.41.1     ->   1aqz+
192.168.41.255   ->   -aqz+
192.168.255.0    ->   +-Wz+
10.0.0.0         ->       a
10.0.0.1,        ->   1+++a
10.0.0.24        ->   o+++a
10.0.0.192       ->  -++++a
10.0.0.193       ->  -1+++a
172.16.0.0       ->     12I
172.80.99.0      ->    oR2I
1.2.3.4          ->   4+M81
80.40.20.10      ->   a52xg
43.86.129.172    ->  0IwloH
43.86.129.197    ->  -5wloH
```

### 48-Bit Values

MAC addresses are usually printed using hexadecimal notation and a colon as separator, which means they 17 characters.
Without the separators, they would still use 12 characters (48 bits divided by 4 bits).
In our base64 variant, they use 1-8 characters (48 bits divided by 6 bits).

```
00:00:00:00:00:00  ->         +
ff:ff:ff:ff:ff:ff  ->         -
c4:c4:c4:c4:c4:c4  ->  Ncj4Ncj4
12:34:56:78:90:12  ->  4zhmu9+i
a1:b2:c3:d4:e5:f6  ->  Erb3RenS
12:34:56:00:00:00  ->  4zhm++++
00:00:00:12:34:56  ->      4zhm
00:1b:44:11:3a:b7  ->   1J44jGT
2c:54:91:88:c9:e3  ->  b5ihycDz
```

## Features

The library currently supports:
* **24-Bit** values (using 32-bit data types), since it uses the full output domain of 4 characters
* **32-Bit** values, with an inefficient extra character for the two most significant bits.
* **48-Bit** values (using 64-bit data types), since again it uses the full output domain of 8 characters.
* **64-Bit** values, with an inefficient extra character for the four most significant bits.
* **128-Bit** values (using two 64-bit values), to support things like UUIDs (two MSBs are inefficient, but the overall length is 22 chars anyway).

The libaries output features are:
* Omitting **leading zeros** (which we encode using ```+```).
* Omitting **leading ones** (which we encode using ```-```), with the downside of being able to omit one less ```+``` character for certain numbers.

## Languages

* **C++** is supported, with a focus on **fast encoding** (and probably fast decoding), to produce the initial data.
* **Scala** is supported, with a focus on **correctness**, assuming less iterated code, more user-driven read/write patterns.
* The encoding and decoding code is short and should be easy enough to port to other languages.
