#include <stdio.h>
#include <conio.h>
#include <dos.h>

#define B3 247
#define Ab3 117
#define D4 294
#define D5 587
#define A4 440
#define Gb4 415
#define G4 392
#define F4 349
#define C4 262
#define B2 494
#define A2 271
#define A3 136
#define Ab2 287
#define C3 228
#define D3 203
#define E3 181
#define F3 169
#define Eb3 192
#define TIME_RUN 65536

//Количество нот

const int MeloidaCount = 26;
//Ноты

int Meloida[MeloidaCount]= {B2, A2, Ab2, A2, C3, D3, C3, B2, C3, E3,
                                    F3, E3, Eb3, E3, B3, A3, Ab3, A3, B3, A3,
                                    Ab3, B3, A3, Ab3, D3, C4};
//Паузы

int MeloidaDelay[MeloidaCount] = { 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
                                                             0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
                                                             0,0 , 0, 0, 0, 1};
//BPM
int MeloidaDuration = 121;

void SoundGeneration();																			//Мелодия
void DivisionCoefficientComputation();															//Опр. коэффициент деления
void StatusWord();																				//Слова состояния каналов
void RandomNumber();																			//RNG

int main(){

        //Выбор функции

        char choice;
        do {

                printf("\n1. Generate sound.\n2. Division coefficient.\n3. Status word.\n4. Random number.\n0. Exit.\n");
                rewind(stdin);
                printf("\nYour choise: ");
                scanf("%c", &choice);

                switch (choice) {

                        case '1': SoundGeneration(); break;
                        case '2': DivisionCoefficientComputation(); break;
                        case '3': StatusWord(); break;
                        case '4': RandomNumber(); break;
                }

        } while (choice != '0');

        return 0;
}

void SoundGeneration() {

        long unsigned base = 1193180;
        int divisionCoefficient;

        //Выбор мелодии
        int count = MeloidaCount;																//Количество нот
        int* frequency = Meloida;																//Массив частот
        int duration = MeloidaDuration;															//Длительность ноты/паузы
        int* delays = MeloidaDelay;																//Массив пауз

        for (int counter = 0; counter < count; counter++){

                outp(0x43, 0xB6);																		//Выбор режима работы
                divisionCoefficient = base / frequency[counter];										//Коэфф деления

                //Загрузка регистра	счетчика таймера
                outp(0x42, divisionCoefficient % 256);													//Младший байт
                divisionCoefficient /= 256;
                outp(0x42, divisionCoefficient);														//Старший байт

                outp(0x61, inp(0x61) | 3);																//Включить динамик
                delay(duration);																		//Ожидание = длительность ноты
                outp(0x61, inp(0x61) & 0xFC);															//Выключить динамик
                delay(delays[counter]* duration);														//Пауза
        }
}

void DivisionCoefficientComputation(){

        unsigned long j;
        int ports[] = { 0x40, 0x41, 0x42 };															//Порты каналов
        int controlWords[] = { 0, 64, 128 };														//Управляющие слова для выбора канала
        unsigned int divisionCoefficientLow, divisionCoefficientHigh, divisionCoefficient, max;

        printf("\n\nCoefficient of division: \n");

        for (int channel = 0; channel < 3; channel++){

                divisionCoefficient = 0;
                max = 0;

                if (channel == 2){

                        outp(0x61, inp(0x61) | 3);															//Включить динамик
                }

                for (j = 0; j < TIME_RUN; j++){

                        outp(0x43, controlWords[channel]);													//Выбор канала
                        divisionCoefficientLow = inp(ports[channel]);										//Младший бит
                        divisionCoefficientHigh = inp(ports[channel]);										//Старший бит
                        divisionCoefficient = divisionCoefficientHigh * 256 + divisionCoefficientLow;		//Коэффициент деления

                        //Выбор максимального
                        if (divisionCoefficient > max){

                                max = divisionCoefficient;
                        }
                }

                if (channel == 2){

                        outp(0x61, inp(0x61) & 0xFC);														//Выключить динамик
                }

                printf("\nChannel %d: %4X\n", channel, max);											//Вывод в 16 формате
        }
}

void StatusWord(){

        printf("\n\nStatus words: \n\n");
        int Channel, state;
        int ports[] = { 0x40, 0x41, 0x42 };															//Порты каналов
        int controlWords[] = { 226, 228, 232 };														//Упр. слова

        for (Channel = 0; Channel < 3; Channel++){

                outp(0x43, controlWords[Channel]);														//Выбор режима работы
                state = inp(ports[Channel]);															//Получение слова состояния

                //Вывод слова состояния в двоичном формате
                printf("\nChannel %d: ", Channel);
                for (int i = 7; i >= 0; i--){

                        printf("%d", state % 2);
                        state /= 2;

                }
                printf("\n");
        }
}

void RandomNumber(){

        char choice;
        unsigned int limit = TIME_RUN - 1, numberLow, numberHigh, number;

        //Установка макс значения
        outp(0x43, 0xB4);																			//Выбор режима работы
        outp(0x42, limit % 256);																	//Младший байт
        limit /= 256;
        outp(0x42, limit);																			//Старший байт
        outp(0x61, inp(0x61) | 1);																	//Установка первого бита в 1

        do{

                printf("\n\n1. Set a limit.\n2. Get a number.\n0. Exit\n");
                fflush(stdin);
                printf("\nYour choise: ");
                scanf("%s", &choice);

                switch (choice){

                        case '1':{

                                do{

                                        printf("\nEnter a limit in range [ 1 ; 65635 ].\n");
                                        fflush(stdin);
                                } while (!scanf("%d", &limit) || limit < 1);

                                //Установка макс значения
                                outp(0x43, 0xB4);																//Выбор режима работы
                                outp(0x42, limit % 256);														//Младший байт
                                limit /= 256;
                                outp(0x42, limit);																//Старший байт
                                outp(0x61, inp(0x61) | 1);														//Установка первого бита в 1
                                break;
                        }

                        case '2':{

                                outp(0x43, 128);																//Выбор 2 канала
                                numberLow = inp(0x42);															//Младший бит
                                numberHigh = inp(0x42);															//Старший бит
                                number = numberHigh * 256 + numberLow;											//Число

                                printf("\nRandom number: %u\n", number);

                                break;
                        }
                }

        } while (choice != '0');

        outp(0x61, inp(0x61) & 0xFC);																//Отключение динамика
}
