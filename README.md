# OpenALRecorder

Приложение, позволяющее записать аудио-поток с микрофона в файл формата WAV.

## Pre-build

Необходимо установить библиотеку OpenAL. Например, в ОС Ubuntu это делается следующей командой:

```console
$ sudo apt-get install libopenal-dev
```

## Build

```console
$ git clone https://github.com/lisa-bella97/OpenALRecorder
$ cd OpenALRecorder
$ mkdir build && cd build
$ cmake .. && make
```

## Run

```
$ ./openALRecorder [--device <device name>]
                   [--channels <1|2>]
                   [--bits <8|16|32>]
                   [--rate <record rate in Hz>]
                   [--time <record time in seconds>]
                   [--file <file name (*.wav)>]
```

Команда ниже запустит программу с опциями по умолчанию (`--device <default_device> --channels 1 --bits 16 --rate 9600 --time 5 --file "default.wav"`):

```console
$ ./openALRecorder
```
