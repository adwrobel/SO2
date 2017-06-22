//Adam Wróbel 200810 Czw. godz. 15:15 Projekt Systemy Operacyjne 2//
#include <iostream>
#include <thread> 
#include <ncurses.h> 
#include <vector> 
#include <cstdlib>
#include <chrono>
#include <mutex>

using namespace std;
const int M=20;//wartosc M moze byc dowolna >0
const int N=10;//wartosc N przy N>=15 wyrzuca blad pamieci. Sprawdzane na innych komputerach
		//ze program dziala nawet dla 20+, ale powyzej tej liczby takze jest blad z pamiecia.
int tab[M][N]; //macierz
int suma=1; //pionek wstawiany do macierzy
mutex kolejka1Guard; //mutex dla kolejki1
mutex kolejka2Guard; //mutex dla kolejki2
mutex tabGuard;	//mutex dla macierzy
vector <int> kolejka1; //pierwsza kolejka liczb
vector <int> kolejka2; //druga kolejka liczb

void producent(){
//Nieskonczona petla generujaca liczby do dwoch kolejek//
	while(true){
		//ustawiamy locka na kolejki
		kolejka1Guard.lock();
		kolejka2Guard.lock();
		//wrzucamy na koniec listy 1 i 2 losowe liczb z zakresu(0...M-1) (0...N-1)
		kolejka1.push_back((rand() % M));
		kolejka2.push_back((rand() % N));
		//ustawiamy unlocka na kolejki
		kolejka1Guard.unlock();
		kolejka2Guard.unlock();
		this_thread::sleep_for(chrono::seconds(1));//opoznienie generowania liczb, aby kolejki sie za bardzo nie zapchaly//
	}
}
void przetwarzacz(){
//lokalne zmienne pomocnicze przy pobieraniu wartosci z kolejek i generowaniu losowych wspolrzednych//
int zm1=0;
int zm2=0;
int zm3=0;
int zm4=0;
this_thread::sleep_for(chrono::seconds(1));//opozniony start przetwarzania
while(true){
	//Pobieranie liczb z losowych wspolrzednych kolejki 1 i 2//
	//jezeli wielkosc jednej z kolejek jest rowna 1 to wartosc pionka
	//rowna sie wartosci 1 elementow w kolejkach
	if(kolejka1.size()==1 || kolejka2.size()==1){
		//ustawiamy locka na kolejki
		kolejka1Guard.lock();
		kolejka2Guard.lock();
		//generujemy wartosc pionka
		suma = kolejka1[0]+kolejka2[0];
		//ustawiamy unlocka na kolejki
		kolejka1Guard.unlock();
		kolejka2Guard.unlock();
	}
	//jezeli wielkosc kolejek jest rowna 0 to pionek otrzymuje wartosc 1
	//nie ma z czego pobierac wartosci//
	if(kolejka1.size()==0 || kolejka2.size()==0){
		suma = 1;
	}
	else{
		//zm1, zm2 - wielkosc kolejek
		zm1 = kolejka1.size();
		zm2= kolejka2.size();
		//zm3, zm4 losujemy z zakresu (0..size-1) liczbe o tych wspolrzednych w kolejce
		zm3=(rand()%zm1);
		zm4=(rand()%zm2);
		//ustawiamy locka na kolejki
		kolejka1Guard.lock();
		kolejka2Guard.lock();
		//generujemy pionek 
		suma = kolejka1[zm3] + kolejka2[zm4];
		//ustawiamy unlocka na kolejki
		kolejka1Guard.unlock();
		kolejka2Guard.unlock();
	}

	//Petla przesuwajaca elementy na macierzy o 1, zaczynajac od ostatniego elementu//
	for(int k=M;k>=0;k--)
	{
		for(int l=N;l>=0; l--){
			//ustawiamy locka na macierz
			tabGuard.lock();
			//jezeli skoncza sie elementy w rzedzie z nastepnego pobieramy liczbe do aktualnego
			if(k!=0 && l==0) {tab[k][l] = tab[k-1][N-1];}
			//tego ifa zostawiamy pustego, pierwszego elementu na macierzy nie przesuwamy, poniewaz robimy to od tylu, a na pierwszy element wchodzi nowa wartosc
			if(k==0 && l==0) {}
			//przesuwanie glowne
			else tab[k][l]=tab[k][l-1];
			//ustawiamy unlocka na macierz
			tabGuard.unlock();
		}
	}
        //ustawiamy locka na macierz
        tabGuard.lock();
        tab[0][0]=suma; //dodanie pionka do macierzy na 1 pozycji//
        //ustawiamy unlocka na macierz
        tabGuard.unlock();
	this_thread::sleep_for(chrono::seconds(1));//opoznienie pobierania i przesuwania pionkow//
	}
}

void ncurses(){
this_thread::sleep_for(chrono::seconds(2)); //opoznienie wystartowania watku wyswietlania macierzy//
while(true){
	clear(); //czyszczenie ekranu//
	//Petla wysylajaca na 'wirtualny' print elementy macierzy//
	for(int n=0; n<M;n++){
		//nowa linia przy rozpoczeciu nowego wiersza
		printw("\n");
		for(int o=0; o<N;o++){
			//ustawiamy locka na macierz
			tabGuard.lock();
			//wirtualny print elementu macierzy
			printw("%d ", tab[n][o]);
			//ustawiamy unlocka na macierz
			tabGuard.unlock();
		}
	}
	refresh(); //odswiezenie wyswietlenia rzeczywistego na konsoli//
	this_thread::sleep_for(chrono::seconds(1));//opoznienie 1 sekundowe wyswietlania//

}
}
int main()
{
initscr();
srand (time(NULL));
auto watekProducent = thread(producent);
auto watekPrzetwarzacz = thread(przetwarzacz);
auto watekNcurses = thread(ncurses);
watekProducent.join();
watekPrzetwarzacz.join();
watekNcurses.join();
getch();
endwin();
return 0;
}
