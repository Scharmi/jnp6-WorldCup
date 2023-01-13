#ifndef WORLDCUP2022_H
#define WORLDCUP2022_H

#include <memory>
#include <string>
#include <list>
#include <vector>

#include <worldcup.h>

class Player {
private:
    uint64_t money;
    std::string const name;
    uint64_t field;
    uint64_t suspension;
public:
    Player(std::string const &name) : name(name){
        this->money = 1000;
        this->suspension = 0;
    }

    bool bankrupt() {
        return money == 0;
    }

    bool waiting() {
        return suspension > 0;
    }

    std::string status() {

    }

    std::string name() {
        
    }

    void pay(uint64_t i) {

    }

    void get() {

    }
};

class BoardField {
private:
    std::string const name;
public:
    virtual ~BoardField() = default;

    virtual void passField();

    virtual void landOnField();
};

class Beginning : public BoardField {
private:
    uint64_t gift = 50;
public:

};

class Goal : public BoardField {
private:
    uint64_t bonus;
public:

};

class Penalty : public BoardField {
private:
    uint64_t fee;
public:  
};

class YellowCard : public BoardField {
private:
    uint64_t suspension;
public:  
};

class Match : public BoardField {
private:
    uint64_t fee;
    int weight;
    uint64_t howManyFees;
public:

};

class Board {
private:
    std::vector<BoardField> fields;
public:

};

class WorldCup2022 : public WorldCup {
public:
    WorldCup2022() {}

    // destruktor
    ~WorldCup2022() {

    }

    // Jeżeli argumentem jest pusty wskaźnik, to nie wykonuje żadnej operacji
    // (ale nie ma błędu).
    void addDie(std::shared_ptr<Die> die) {

    }

    // Dodaje nowego gracza o podanej nazwie.
    void addPlayer(std::string const &name) {

    }

    // Konfiguruje tablicę wyników. Domyślnie jest skonfigurowana tablica
    // wyników, która nic nie robi.
    void setScoreBoard(std::shared_ptr<ScoreBoard> scoreboard) {

    }

    // Przeprowadza rozgrywkę co najwyżej podanej liczby rund (rozgrywka może
    // skończyć się wcześniej).
    // Jedna runda obejmuje po jednym ruchu każdego gracza.
    // Gracze ruszają się w kolejności, w której zostali dodani.
    // Na początku każdej rundy przekazywana jest informacja do tablicy wyników
    // o początku rundy (onRound), a na zakończenie tury gracza informacje
    // podsumowujące dla każdego gracza (onTurn).
    // Rzuca TooManyDiceException, jeśli jest zbyt dużo kostek.
    // Rzuca TooFewDiceException, jeśli nie ma wystarczającej liczby kostek.
    // Rzuca TooManyPlayersException, jeśli jest zbyt dużo graczy.
    // Rzuca TooFewPlayersException, jeśli liczba graczy nie pozwala na
    // rozpoczęcie gry.
    // Wyjątki powinny dziedziczyć po std::exception.
    void play(unsigned int rounds) {
        
    }

private:
    std::shared_ptr<ScoreBoard> scoreboard;
    std::vector<Die> dies;
    std::vector<Player> players;
};

#endif