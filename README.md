# OpenALRecorder

Приложение, позволяющее записать аудио-поток с микрофона в файл формата WAV.

## Pre-build

Необходимо установить библиотеку OpenAl. Например, в ОС Ubuntu это делается следующей командой:

```console
$ sudo apt-get install libopenal-dev
```

## Build

```console
$ git clone https://github.com/lisa-bella97/OpenALRecorder
$ cd OpenALRecorder
$ mkdir build && cd build
$ cmake && make
```

## Run

```
$ ./openAlRecorder [--device <device name>]
                   [--channels <1|2>]
                   [--bits <8|16|32>]
                   [--rate <record rate in Hz>]
                   [--time <record time in seconds>]
                   [--file <file name (*.wav)>]
```

Команда ниже запустит программу с опциями по умолчанию (`--device <default_device> --channels 1 --bits 16 --rate 44100 --time 5 --file "../saved/default.wav"`):

```console
$ ./openAlRecorder
```
